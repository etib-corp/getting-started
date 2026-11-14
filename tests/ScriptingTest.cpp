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
 * @brief Runs a script through the host bindings, without a window.
 *
 * Usage: scripting_test <script-directory> <module>
 *
 * With the module `app`, the test checks the application's real script: it
 * loads, proposes valid settings and survives a few frames. With the module
 * `fixture`, it checks what tests/scripts/fixture.py does with each binding.
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <mais/ScriptRuntime.hpp>

#include "scripting/Scripting.hpp"

namespace
{
	int failures = 0;

	/**
	 * @brief Record a failed expectation without stopping the test.
	 * @param condition What must hold.
	 * @param message What to print when it does not.
	 */
	void expect(bool condition, std::string_view message)
	{
		if (!condition) {
			std::cerr << "FAILED: " << message << std::endl;
			++failures;
		}
	}

	/**
	 * @brief Record a failed mais call, with its Python traceback.
	 * @param error The error returned by the call.
	 * @param what The call.
	 */
	void expectOk(const mais::Error &error, std::string_view what)
	{
		if (error) {
			std::cerr << "FAILED: " << what << ": " << error.toString()
					  << std::endl;
			++failures;
		}
	}

	/**
	 * @brief Check validate() on its own.
	 */
	void testValidation(void)
	{
		app::scripting::Settings settings;

		expect(app::scripting::validate(settings).empty(),
			   "the default settings are valid");

		settings.width = 0;
		expect(app::scripting::validate(settings).size() == 1,
			   "a width of 0 is rejected");

		settings.windowTitle.clear();
		expect(app::scripting::validate(settings).size() == 2,
			   "an empty title is rejected too");
	}
}	 // namespace

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::cerr << "usage: " << argv[0] << " <script-directory> <module>"
				  << std::endl;
		return EXIT_FAILURE;
	}
	const std::string directory = argv[1];
	const std::string module	= argv[2];
	const bool isFixture		= module == "fixture";

	testValidation();

	// Declared before the runtime: Python borrows it.
	app::scripting::Services services;
	mais::ScriptRuntime runtime;

	expectOk(app::scripting::registerBindings(runtime, services),
			 "registerBindings");
	expectOk(runtime.initialize(), "initialize");
	expectOk(runtime.addSearchPath(directory), "addSearchPath");
	expectOk(runtime.loadModule(module), "loadModule");

	if (failures == 0) {
		const mais::CallReport configured = runtime.callOptional(
			mais::FailurePolicy::Stop, module, "configure",
			{ app::scripting::settingsArgument(services.settings) });
		expectOk(configured.error, "configure");
		expect(app::scripting::validate(services.settings).empty(),
			   "configure proposes valid settings");

		expectOk(
			runtime
				.callOptional(mais::FailurePolicy::Stop, module, "on_start",
							  { app::scripting::frameArgument(services.frame) })
				.error,
			"on_start");

		for (int frame = 0; frame < 3; ++frame) {
			services.frame.deltaSeconds = 1.0 / 60.0;
			services.frame.elapsedSeconds += services.frame.deltaSeconds;
			services.frame.clicks = static_cast<std::uint64_t>(frame);
			expectOk(runtime
						 .callOptional(
							 mais::FailurePolicy::Stop, module, "on_update",
							 { app::scripting::frameArgument(services.frame) })
						 .error,
					 "on_update");
			++services.frame.index;
		}

		expectOk(runtime
					 .callOptional(
						 mais::FailurePolicy::Continue, module, "on_shutdown",
						 { app::scripting::frameArgument(services.frame) })
					 .error,
				 "on_shutdown");

		if (isFixture) {
			expect(services.settings.windowTitle == "fixture",
				   "configure writes window_title");
			expect(services.settings.width == 640
					   && services.settings.height == 480,
				   "configure writes width and height");
			expect(services.frame.status == "frame 2, 2 clicks",
				   "on_update reads index and clicks and writes status");
			expect(services.quitRequested,
				   "host.request_quit() sets the quit flag");

			// A read-only field must stay read-only from Python.
			const mais::Error readOnly =
				runtime.call(module, "write_read_only_field",
							 { app::scripting::frameArgument(services.frame) });
			expect(static_cast<bool>(readOnly),
				   "writing a read-only field raises");
			expect(services.frame.clicks == 2,
				   "a read-only field keeps its value");
		} else {
			expect(!services.frame.status.empty(),
				   "on_update writes a status line");
			expect(!services.quitRequested,
				   "the script does not quit on its own");
		}
	}

	expectOk(runtime.shutdown(), "shutdown");

	if (failures != 0) {
		std::cerr << failures << " expectation(s) failed" << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "all expectations passed (" << module << ")" << std::endl;
	return EXIT_SUCCESS;
}
