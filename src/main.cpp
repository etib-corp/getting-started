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
 * @brief The host: start-up order and frame loop.
 *
 * The three libraries meet here and nowhere else:
 * - mais runs the Python hooks of scripts/app.py,
 * - evan opens the window and renders,
 * - guillaume owns the scenes and the widgets.
 *
 * Order of a session:
 * 1. start Python and load the script,
 * 2. `configure(settings)`: the script proposes the window settings,
 * 3. validate the proposal, then create the window and the engine with it,
 * 4. `on_start(context)`,
 * 5. each frame: `on_update(context)`, then one guillaume frame,
 * 6. `on_shutdown(context)`, then stop Python.
 */

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <evan/Engine.hpp>
#include <evan/IPlatform.hpp>

#if defined(__APPLE__)
	#include <evan/glfw/platform/MacOsDesktopPlatform.hpp>
#elif defined(__linux__)
	#include <evan/glfw/platform/LinuxDesktopPlatform.hpp>
#elif defined(_WIN32)
	#include <evan/glfw/platform/WindowsPlatform.hpp>
#endif

#include <guillaume/application.hpp>

#include <mais/ScriptRuntime.hpp>

#include <utility/ressource_provider.hpp>
#include <utility/system_io/default_system_io.hpp>

#include "AppContext.hpp"
#include "scenes/HomeScene.hpp"
#include "scripting/Scripting.hpp"

namespace
{
	/// Python module holding the hooks: scripts/app.py.
	constexpr std::string_view scriptModule = "app";

	/// Directory of the scripts, relative to the runtime directory.
	constexpr const char *scriptDirectory = "scripts";

	/**
	 * @brief Make the directory of the executable the working directory.
	 *
	 * Shaders, fonts and scripts are loaded by relative path and are copied
	 * next to the executable by the build, so the application has to run from
	 * there. This makes `./build/bin/getting_started` work from any directory.
	 * @param executable argv[0].
	 */
	void enterRuntimeDirectory(const char *executable)
	{
		std::error_code error;

		if (std::filesystem::exists("shaders", error)
			|| executable == nullptr) {
			return;
		}
		const auto directory =
			std::filesystem::absolute(executable, error).parent_path();
		if (!error && std::filesystem::exists(directory / "shaders", error)) {
			std::filesystem::current_path(directory, error);
		}
	}

	/**
	 * @brief Create the desktop platform of the host system.
	 * @param settings The validated window settings.
	 * @return The platform, which owns the window.
	 */
	std::shared_ptr<evan::IPlatform>
		makePlatform(const app::scripting::Settings &settings)
	{
		const auto width  = static_cast<std::uint32_t>(settings.width);
		const auto height = static_cast<std::uint32_t>(settings.height);

#if defined(__APPLE__)
		return std::make_shared<evan::MacOsDesktopPlatform>(
			settings.windowTitle, width, height);
#elif defined(__linux__)
		return std::make_shared<evan::LinuxDesktopPlatform>(
			settings.windowTitle, width, height);
#elif defined(_WIN32)
		return std::make_shared<evan::WindowsDesktopPlatform>(
			settings.windowTitle, width, height);
#else
	#error "Unsupported platform: evan's GLFW backend is desktop only"
#endif
	}

	/**
	 * @brief Print a script error with its Python traceback.
	 * @param what The step that failed.
	 * @param error The error reported by mais.
	 */
	void reportScriptError(std::string_view what, const mais::Error &error)
	{
		std::cerr << "[" APP_NAME "] " << what << ": " << error.toString()
				  << std::endl;
	}

	/**
	 * @brief Read the frame limit used by automated runs.
	 *
	 * Set GETTING_STARTED_MAX_FRAMES to close the application after that many
	 * frames, for instance in a smoke test.
	 * @return The limit, or 0 for no limit.
	 */
	std::uint64_t readFrameLimit(void)
	{
		const char *value = std::getenv("GETTING_STARTED_MAX_FRAMES");

		if (value == nullptr) {
			return 0;
		}
		try {
			return std::stoull(value);
		} catch (const std::exception &) {
			return 0;
		}
	}

	/**
	 * @brief Start Python, expose the `host` module and load the script.
	 * @param runtime The runtime to start.
	 * @param services The state the script can reach.
	 * @return True when the script is loaded and its hooks can be called.
	 */
	bool startScripting(mais::ScriptRuntime &runtime,
						app::scripting::Services &services)
	{
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
		if (mais::Error error = runtime.addSearchPath(scriptDirectory); error) {
			reportScriptError("addSearchPath", error);
			return false;
		}
		if (mais::Error error = runtime.loadModule(std::string(scriptModule));
			error) {
			reportScriptError("loadModule", error);
			return false;
		}
		return true;
	}

	/**
	 * @brief Let the script propose the window settings, then validate them.
	 *
	 * An invalid proposal is reported and replaced by the defaults, so the
	 * engine never starts with values the host did not accept.
	 * @param runtime The running script runtime.
	 * @param settings The settings; holds the defaults on entry.
	 * @return False when `configure` raised: the proposal may be half
	 * written, so the application must not start.
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
			return false;
		}

		const std::vector<std::string> problems =
			app::scripting::validate(settings);
		if (!problems.empty()) {
			for (const std::string &problem: problems) {
				std::cerr << "[" APP_NAME "] rejected setting: " << problem
						  << std::endl;
			}
			settings = defaults;
		}
		return true;
	}

	/**
	 * @brief Run the window and the frame loop until the user quits.
	 * @param runtime The running script runtime.
	 * @param context The shared application context.
	 * @return The process exit code.
	 */
	int runSession(mais::ScriptRuntime &runtime, app::Context &context)
	{
		using Clock = std::chrono::steady_clock;

		app::scripting::Services &services = context.scripting;

		utility::DefaultSystemIO systemIo;
		auto resources = std::make_shared<utility::RessourceProvider>(systemIo);
		auto engine	   = std::make_unique<evan::Engine>(
			   resources, makePlatform(services.settings));

		// List every scene of the application here; the first one is shown
		// at start-up.
		guillaume::Application<app::scenes::HomeScene> application(
			resources, std::move(engine));

		// A session whose start hook failed does not enter its loop.
		const mais::CallReport started = runtime.callOptional(
			mais::FailurePolicy::Stop, scriptModule, "on_start",
			{ app::scripting::frameArgument(services.frame) });
		if (started) {
			reportScriptError("on_start", started.error);
		}
		if (started.stopRequested) {
			return EXIT_FAILURE;
		}

		const std::uint64_t frameLimit = readFrameLimit();
		const Clock::time_point start  = Clock::now();
		Clock::time_point previous	   = start;
		std::string shownStatus;

		while (!application.shouldQuit() && !services.quitRequested) {
			const Clock::time_point now = Clock::now();

			services.frame.deltaSeconds =
				std::chrono::duration<double>(now - previous).count();
			services.frame.elapsedSeconds =
				std::chrono::duration<double>(now - start).count();
			previous = now;

			// The script runs first, so what it writes is drawn this frame.
			// ReportOnce prints the first failure of a streak and keeps the
			// loop alive: a broken hook must not close the window.
			const mais::CallReport updated = runtime.callOptional(
				mais::FailurePolicy::ReportOnce, scriptModule, "on_update",
				{ app::scripting::frameArgument(services.frame) });
			if (updated) {
				reportScriptError("on_update", updated.error);
			}

			if (context.showStatus && services.frame.status != shownStatus) {
				shownStatus = services.frame.status;
				context.showStatus(shownStatus);
			}

			// One guillaume frame: events, layout, render, present.
			application.routine();

			++services.frame.index;
			if (frameLimit != 0 && services.frame.index >= frameLimit) {
				break;
			}
		}

		// Continue: a failing shutdown hook never prevents the exit.
		const mais::CallReport stopped = runtime.callOptional(
			mais::FailurePolicy::Continue, scriptModule, "on_shutdown",
			{ app::scripting::frameArgument(services.frame) });
		if (stopped) {
			reportScriptError("on_shutdown", stopped.error);
		}
		return EXIT_SUCCESS;
	}

}	 // namespace

int main(int, char **argv)
{
	enterRuntimeDirectory(argv[0]);

	// The context is created before the runtime and outlives it: Python
	// borrows the objects it holds.
	app::Context &context = app::context();
	mais::ScriptRuntime runtime;
	int exitCode = EXIT_FAILURE;

	if (startScripting(runtime, context.scripting)
		&& configure(runtime, context.scripting.settings)) {
		try {
			exitCode = runSession(runtime, context);
		} catch (const std::exception &exception) {
			std::cerr << "[" APP_NAME "] fatal: " << exception.what()
					  << std::endl;
		}
	}

	// The window and the scenes are gone; stop Python last.
	if (mais::Error error = runtime.shutdown(); error) {
		reportScriptError("shutdown", error);
	}
	return exitCode;
}
