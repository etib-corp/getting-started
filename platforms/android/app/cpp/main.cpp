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
 * - there is no Python here, so the host writes the status line itself.
 */

#include <jni.h>

#include <android_native_app_glue.h>

#include <android/log.h>
#include <android/looper.h>
#include <android/native_activity.h>

#include <vulkan/vulkan.h>

#include <openxr/openxr_platform.h>

#include <exception>
#include <memory>
#include <utility>

#include <evan/Engine.hpp>
#include <evan/openxr/platform/AndroidXrPlatform.hpp>

#include <utility/ressource_provider.hpp>
#include <utility/system_io/android_system_io.hpp>

#include "AppContext.hpp"
#include "Application.hpp"
#include "Frame.hpp"

namespace
{
	/// Tag of the application's own lines in `adb logcat`.
	constexpr const char *logTag = APP_NAME;

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
	 */
	void runSession(struct android_app *androidApp)
	{
		app::Context &context = app::context();

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

		app::FrameClock clock;

		while (androidApp->destroyRequested == 0) {
			pollAndroidEvents(androidApp, platform->_appState);
			if (androidApp->destroyRequested != 0) {
				break;
			}
			if (application.shouldQuit() || context.scripting.quitRequested) {
				// Ask Android to close the activity; the loop ends when the
				// destroy request arrives.
				context.scripting.quitRequested = false;
				ANativeActivity_finish(androidApp->activity);
				continue;
			}

			clock.tick(context.scripting.frame);

			// No script on this platform: the host writes what
			// scripts/app.py writes on desktop.
			app::writeDefaultStatus(context.scripting.frame);
			app::presentStatus(context);

			// One guillaume frame: events, layout, render, present.
			application.routine();
		}

		androidApp->userData = nullptr;
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

	try {
		runSession(androidApp);
	} catch (const std::exception &exception) {
		__android_log_print(ANDROID_LOG_FATAL, logTag, "fatal: %s",
							exception.what());
		ANativeActivity_finish(androidApp->activity);
	}

	androidApp->activity->vm->DetachCurrentThread();
}
}
