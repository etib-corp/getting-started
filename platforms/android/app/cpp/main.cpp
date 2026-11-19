/*
 Copyright (c) 2026 ETIB Corporation

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

/**
 * @file
 * @brief The Android host: entry point and frame loop on a Meta headset.
 *
 * Same application as the desktop host (platforms/desktop/main.cpp): the
 * scenes and the shared context come from src/. What differs:
 * - evan renders through OpenXR instead of a GLFW window,
 * - files are read from the APK assets,
 * - the Android activity lifecycle drives the loop,
 * - Python is the build published on python.org, packaged in the APK: its
 *   standard library and the scripts are copied from the assets to the
 *   application's private directory before the interpreter starts.
 *
 * Order of a session, as on desktop:
 * 1. copy the Python files, start Python and load the script,
 * 2. `configure(settings)`: validated, but a headset has no window to size,
 * 3. create the OpenXR platform, the engine and the scenes,
 * 4. `on_start(context)`,
 * 5. each frame: `on_update(context)`, then one guillaume frame,
 * 6. `on_shutdown(context)`, then stop Python.
 *
 * A headset application that does not start shows nothing, so a script
 * problem never prevents the session here: it is logged, the hooks are no
 * longer called and the host writes the status line itself.
 */

#include <jni.h>

#include <android_native_app_glue.h>

#include <android/log.h>
#include <android/looper.h>
#include <android/native_activity.h>

#include <vulkan/vulkan.h>

#include <openxr/openxr_platform.h>

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <evan/Engine.hpp>
#include <evan/openxr/platform/AndroidXrPlatform.hpp>

#include <mais/ScriptRuntime.hpp>

#include <utility/ressource_provider.hpp>
#include <utility/system_io/android_system_io.hpp>

#include "AppContext.hpp"
#include "Application.hpp"
#include "AssetTree.hpp"
#include "Frame.hpp"
#include "scripting/Scripting.hpp"

namespace
{
	/// Tag of the application's own lines in `adb logcat`.
	constexpr const char *logTag = APP_NAME;

	/// Python module holding the hooks: scripts/app.py.
	constexpr std::string_view scriptModule = "app";

	/// Asset directory of the scripts, and their directory once copied.
	constexpr const char *scriptDirectory = "scripts";

	/// Directory of the Python home once copied, and its asset directory.
	constexpr const char *pythonDirectory = "python";

	/**
	 * @brief Log a script error with its Python traceback.
	 * @param what The step that failed.
	 * @param error The error reported by mais.
	 */
	void reportScriptError(std::string_view what, const mais::Error &error)
	{
		__android_log_print(ANDROID_LOG_ERROR, logTag, "%.*s: %s",
							static_cast<int>(what.size()), what.data(),
							error.toString().c_str());
	}

	/**
	 * @brief Copy the Python files out of the APK and point Python at them.
	 *
	 * The standard library and the scripts are assets; Python needs files.
	 * They are copied to the application's private directory, once per
	 * installed version, and `PYTHONHOME` tells the interpreter where its
	 * standard library is.
	 * @param androidApp The native activity glue.
	 * @param scripts Receives the directory the scripts were copied to.
	 * @return False when the files could not be copied.
	 */
	bool preparePythonFiles(struct android_app *androidApp,
							std::filesystem::path &scripts)
	{
		const ANativeActivity *activity = androidApp->activity;

		if (activity->internalDataPath == nullptr) {
			__android_log_print(ANDROID_LOG_ERROR, logTag,
								"python: the activity has no data directory");
			return false;
		}
		const std::filesystem::path data = activity->internalDataPath;
		const std::filesystem::path home = data / pythonDirectory;

		scripts = data / scriptDirectory;

		const std::string problems[] = {
			app::android::extractAssetTree(activity->assetManager,
										   std::string(pythonDirectory)
											   + "/" APP_PYTHON_STDLIB_SUBDIR,
										   home / APP_PYTHON_STDLIB_SUBDIR),
			app::android::extractAssetTree(activity->assetManager,
										   scriptDirectory, scripts),
		};
		for (const std::string &problem: problems) {
			if (!problem.empty()) {
				__android_log_print(ANDROID_LOG_ERROR, logTag, "python: %s",
									problem.c_str());
				return false;
			}
		}

		// Workaround for https://github.com/etib-corp/mais/issues/39: mais
		// has no parameter for the Python home, the environment is the only
		// way to give it.
		setenv("PYTHONHOME", home.c_str(), 1);

		// Android only sets TMPDIR from API level 33; `tempfile` needs it.
		std::error_code error;
		const std::filesystem::path temporary = data / "tmp";
		std::filesystem::create_directories(temporary, error);
		setenv("TMPDIR", temporary.c_str(), 0);
		return true;
	}

	/**
	 * @brief Start Python, expose the `host` module and load the script.
	 * @param runtime The runtime to start.
	 * @param services The state the script can reach.
	 * @param androidApp The native activity glue.
	 * @return True when the script is loaded and its hooks can be called.
	 */
	bool startScripting(mais::ScriptRuntime &runtime,
						app::scripting::Services &services,
						struct android_app *androidApp)
	{
		std::filesystem::path scripts;

		if (!preparePythonFiles(androidApp, scripts)) {
			return false;
		}
		// Bindings are queued before Python starts.
		if (mais::Error error =
				app::scripting::registerBindings(runtime, services);
			error) {
			reportScriptError("registerBindings", error);
			return false;
		}
		if (mais::Error error = runtime.initialize(); error) {
			reportScriptError("initialize", error);
			return false;
		}
		if (mais::Error error = runtime.addSearchPath(scripts.string());
			error) {
			reportScriptError("addSearchPath", error);
			return false;
		}
		if (mais::Error error = runtime.loadModule(std::string(scriptModule));
			error) {
			reportScriptError("loadModule", error);
			return false;
		}
		__android_log_print(ANDROID_LOG_INFO, logTag,
							"python: script '%s' loaded from %s",
							std::string(scriptModule).c_str(), scripts.c_str());
		return true;
	}

	/**
	 * @brief Let the script propose the settings, then validate them.
	 *
	 * The hook is called so a script behaves the same on every platform.
	 * The window settings themselves are not used: the headset has no
	 * window.
	 * @param runtime The running script runtime.
	 * @param settings The settings; holds the defaults on entry.
	 * @return False when `configure` raised: the hooks must not be called.
	 */
	bool configure(mais::ScriptRuntime &runtime,
				   app::scripting::Settings &settings)
	{
		const app::scripting::Settings defaults = settings;

		const mais::CallReport report = runtime.callOptional(
			mais::FailurePolicy::Stop, scriptModule, "configure",
			{ app::scripting::settingsArgument(settings) });
		if (report) {
			reportScriptError("configure", report.error);
		}
		if (report.stopRequested) {
			settings = defaults;
			return false;
		}

		const std::vector<std::string> problems =
			app::scripting::validate(settings);
		if (!problems.empty()) {
			for (const std::string &problem: problems) {
				__android_log_print(ANDROID_LOG_WARN, logTag,
									"rejected setting: %s", problem.c_str());
			}
			settings = defaults;
		}
		return true;
	}

	/**
	 * @brief Follow the activity lifecycle.
	 *
	 * evan reads the resumed and paused flags to know when the OpenXR session
	 * may run. `userData` points at the platform's AndroidAppState.
	 * @param androidApp The native activity glue.
	 * @param command The lifecycle command (APP_CMD_*).
	 */
	void handleCommand(struct android_app *androidApp, int32_t command)
	{
		auto *state = static_cast<evan::AndroidXrPlatform::AndroidAppState *>(
			androidApp->userData);

		if (state == nullptr) {
			return;
		}
		switch (command) {
			case APP_CMD_RESUME:
				state->_resumed = true;
				state->_paused	= false;
				break;
			case APP_CMD_PAUSE:
				state->_resumed = false;
				state->_paused	= true;
				break;
			default:
				break;
		}
	}

	/**
	 * @brief Handle the pending Android events.
	 *
	 * Blocks while the activity is paused, so the application uses no CPU in
	 * the background, and returns at once while it is resumed.
	 * @param androidApp The native activity glue.
	 * @param state The lifecycle flags updated by handleCommand().
	 */
	void
		pollAndroidEvents(struct android_app *androidApp,
						  const evan::AndroidXrPlatform::AndroidAppState &state)
	{
		for (;;) {
			int events						   = 0;
			struct android_poll_source *source = nullptr;
			const int timeoutMilliseconds =
				(!state._resumed && androidApp->destroyRequested == 0) ? -1 : 0;

			if (ALooper_pollOnce(timeoutMilliseconds, nullptr, &events,
								 reinterpret_cast<void **>(&source))
				< 0) {
				break;
			}
			if (source != nullptr) {
				source->process(androidApp, source);
			}
		}
	}

	/**
	 * @brief Create the engine and the scenes, then run the frame loop.
	 * @param androidApp The native activity glue.
	 * @param runtime The script runtime.
	 * @param context The shared application context.
	 * @param scripted True when the script is loaded: its hooks are called.
	 */
	void runSession(struct android_app *androidApp,
					mais::ScriptRuntime &runtime, app::Context &context,
					bool scripted)
	{
		app::scripting::Services &services = context.scripting;

		// Shaders, fonts and the application's assets are in the APK.
		utility::AndroidSystemIO systemIo(androidApp->activity->assetManager);
		auto resources = std::make_shared<utility::RessourceProvider>(systemIo);

		evan::AndroidXrPlatform::AndroidPlatformData platformData {
			.applicationVM		 = androidApp->activity->vm,
			.applicationActivity = androidApp->activity->clazz,
			.androidApp			 = androidApp
		};
		auto platform = std::make_shared<evan::AndroidXrPlatform>(platformData);

		androidApp->userData = &platform->_appState;
		androidApp->onAppCmd = handleCommand;

		// For passthrough and hand tracking, enable evan's AR extensions here
		// (platform->setOptionArExtensionsEnabled(true)) and declare the
		// matching features in AndroidManifest.xml.

		// The scenes are listed in src/Application.hpp.
		app::Application application(
			resources, std::make_unique<evan::Engine>(resources, platform));

		if (scripted) {
			// A script whose start hook failed gets no other call.
			const mais::CallReport started = runtime.callOptional(
				mais::FailurePolicy::Stop, scriptModule, "on_start",
				{ app::scripting::frameArgument(services.frame) });
			if (started) {
				reportScriptError("on_start", started.error);
			}
			scripted = !started.stopRequested;
		}

		app::FrameClock clock;

		while (androidApp->destroyRequested == 0) {
			pollAndroidEvents(androidApp, platform->_appState);
			if (androidApp->destroyRequested != 0) {
				break;
			}
			if (application.shouldQuit() || services.quitRequested) {
				// Ask Android to close the activity; the loop ends when the
				// destroy request arrives.
				services.quitRequested = false;
				ANativeActivity_finish(androidApp->activity);
				continue;
			}

			clock.tick(services.frame);

			if (scripted) {
				// The script runs first, so what it writes is drawn this
				// frame. ReportOnce logs the first failure of a streak and
				// keeps the loop alive.
				const mais::CallReport updated = runtime.callOptional(
					mais::FailurePolicy::ReportOnce, scriptModule, "on_update",
					{ app::scripting::frameArgument(services.frame) });
				if (updated) {
					reportScriptError("on_update", updated.error);
				}
			} else {
				// No script: the host writes what scripts/app.py writes.
				app::writeDefaultStatus(services.frame);
			}
			app::presentStatus(context);

			// One guillaume frame: events, layout, render, present.
			application.routine();
		}

		androidApp->userData = nullptr;

		if (scripted) {
			// Continue: a failing shutdown hook never prevents the exit.
			const mais::CallReport stopped = runtime.callOptional(
				mais::FailurePolicy::Continue, scriptModule, "on_shutdown",
				{ app::scripting::frameArgument(services.frame) });
			if (stopped) {
				reportScriptError("on_shutdown", stopped.error);
			}
		}
	}

}	 // namespace

extern "C" {

/**
 * @brief Entry point called by the native activity glue on its own thread.
 * @param androidApp The native activity glue.
 */
void android_main(struct android_app *androidApp)
{
	JNIEnv *environment = nullptr;

	androidApp->activity->vm->AttachCurrentThread(&environment, nullptr);

	{
		// The context is created before the runtime and outlives it: Python
		// borrows the objects it holds. Python runs on this thread only.
		app::Context &context = app::context();
		mais::ScriptRuntime runtime;

		const bool scripted =
			startScripting(runtime, context.scripting, androidApp)
			&& configure(runtime, context.scripting.settings);
		if (!scripted) {
			__android_log_print(ANDROID_LOG_WARN, logTag,
								"running without the Python script");
		}

		try {
			runSession(androidApp, runtime, context, scripted);
		} catch (const std::exception &exception) {
			__android_log_print(ANDROID_LOG_FATAL, logTag, "fatal: %s",
								exception.what());
			ANativeActivity_finish(androidApp->activity);
		}

		// The engine and the scenes are gone; stop Python last.
		if (runtime.isRunning()) {
			if (mais::Error error = runtime.shutdown(); error) {
				reportScriptError("shutdown", error);
			}
		}
	}

	androidApp->activity->vm->DetachCurrentThread();
}
}
