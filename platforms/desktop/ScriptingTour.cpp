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
 * @brief A tour of the maïs runtime: one block per feature.
 *
 * Each block calls the runtime the way a host would, prints what it got and
 * checks it. The Python side is scripts/features.py. Read this file from top
 * to bottom next to docs/mais.md.
 */

#include "ScriptingTour.hpp"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <mais/ScriptRuntime.hpp>

#include "scripting/Scripting.hpp"

namespace app::desktop
{
	namespace
	{
		/// Python module of the tour: scripts/features.py.
		constexpr std::string_view module = "features";

		/// Number of failed checks.
		int failures = 0;

		/**
		 * @brief Print one line of the tour and record a failed check.
		 * @param passed What must hold.
		 * @param feature The feature being shown.
		 * @param observed What the runtime answered.
		 */
		void check(bool passed, std::string_view feature,
				   std::string_view observed)
		{
			std::cout << "[tour] " << (passed ? "ok    " : "FAILED") << " "
					  << feature << ": " << observed << std::endl;
			if (!passed) {
				++failures;
			}
		}

		/**
		 * @brief Name of the code of an error.
		 * @param error The error.
		 * @return For example "FunctionNotFound".
		 */
		std::string codeName(const mais::Error &error)
		{
			return std::string(mais::errorCodeName(error.code()));
		}

		/**
		 * @brief Join strings with ", ".
		 * @param values The strings.
		 * @return The joined text.
		 */
		std::string join(const std::vector<std::string> &values)
		{
			std::string text;

			for (const std::string &value: values) {
				text += (text.empty() ? "" : ", ") + value;
			}
			return text;
		}

		/**
		 * @brief Interpreter lifecycle and its misuse.
		 * @param runtime A runtime that is not started yet.
		 * @param services The state the `host` module gives access to.
		 * @return True when Python is running.
		 */
		bool tourLifecycle(mais::ScriptRuntime &runtime,
						   scripting::Services &services)
		{
			// Using the runtime before initialize() is an error code, not a
			// crash.
			const mais::Error early = runtime.loadModule(std::string(module));
			check(early.code() == mais::ErrorCode::NotInitialized,
				  "loadModule() before initialize()", codeName(early));

			// The `host` module is queued before Python starts...
			const mais::Error bound =
				scripting::registerBindings(runtime, services);
			check(bound.isOk(), "registerModule(\"host\") before initialize()",
				  codeName(bound));

			// ...and created by initialize().
			const mais::Error started = runtime.initialize();
			check(started.isOk() && runtime.isRunning(), "initialize()",
				  started.isOk() ? "Python is running" : started.toString());

			const mais::Error twice = runtime.initialize();
			check(twice.code() == mais::ErrorCode::AlreadyInitialized,
				  "initialize() twice", codeName(twice));
			return started.isOk();
		}

		/**
		 * @brief Search paths and module loading.
		 * @param runtime The running runtime.
		 * @return True when scripts/features.py is loaded.
		 */
		bool tourLoading(mais::ScriptRuntime &runtime)
		{
			const mais::Error path = runtime.addSearchPath("scripts");
			check(path.isOk(), "addSearchPath(\"scripts\")",
				  join(runtime.searchPaths()));

			const mais::Error noPath =
				runtime.addSearchPath("no-such-directory");
			check(noPath.code() == mais::ErrorCode::ScriptNotFound,
				  "addSearchPath() of a missing directory", codeName(noPath));

			const mais::Error loaded = runtime.loadModule(std::string(module));
			check(loaded.isOk() && runtime.isModuleLoaded(std::string(module)),
				  "loadModule(\"features\")",
				  loaded.isOk() ? "loaded" : loaded.toString());

			const mais::Error noModule = runtime.loadModule("no_such_module");
			check(noModule.code() == mais::ErrorCode::ModuleNotFound,
				  "loadModule() of a missing module", codeName(noModule));
			return loaded.isOk();
		}

		/**
		 * @brief Look at a module without running it.
		 * @param runtime The running runtime.
		 */
		void tourIntrospection(mais::ScriptRuntime &runtime)
		{
			// Public callables, sorted. `_check` is private and `host` and
			// `time` are modules: none of them is listed.
			const auto functions = runtime.listFunctions(module);
			check(functions.hasValue() && functions.value().size() == 12
					  && functions.value().front() == "answer",
				  "listFunctions()",
				  functions.hasValue() ? join(functions.value())
									   : functions.error().toString());

			const auto present = runtime.hasFunction(module, "answer");
			const auto absent  = runtime.hasFunction(module, "on_update");
			check(present.hasValue() && present.value() && absent.hasValue()
					  && !absent.value(),
				  "hasFunction()", "answer: yes, on_update: no");

			const auto unknown = runtime.hasFunction("never_loaded", "answer");
			check(unknown.error().code() == mais::ErrorCode::ModuleNotFound,
				  "hasFunction() on a module that was not loaded",
				  codeName(unknown.error()));
		}

		/**
		 * @brief Arguments and typed return values.
		 * @param runtime The running runtime.
		 * @param services The state handed to the script as a native object.
		 */
		void tourValues(mais::ScriptRuntime &runtime,
						scripting::Services &services)
		{
			// The four plain argument types, and a string read back.
			const auto described = runtime.call<std::string>(
				module, "describe",
				{ mais::ScriptArgument::integer(7),
				  mais::ScriptArgument::number(0.5),
				  mais::ScriptArgument::boolean(true),
				  mais::ScriptArgument::string("hi") });
			check(described.hasValue()
					  && described.value()
						  == "int float bool str: 7 0.5 True hi",
				  "integer, number, boolean and string arguments",
				  described.hasValue() ? described.value()
									   : described.error().toString());

			// The four return types.
			const auto integer = runtime.call<std::int64_t>(module, "answer");
			check(integer.hasValue() && integer.value() == 42,
				  "call<std::int64_t>(\"answer\")",
				  integer.hasValue() ? std::to_string(integer.value())
									 : integer.error().toString());

			const auto number = runtime.call<double>(module, "ratio");
			check(number.hasValue() && number.value() == 0.5,
				  "call<double>(\"ratio\")",
				  number.hasValue() ? std::to_string(number.value())
									: number.error().toString());

			const auto flag = runtime.call<bool>(module, "is_ready");
			check(flag.hasValue() && flag.value(), "call<bool>(\"is_ready\")",
				  flag.hasValue() ? (flag.value() ? "true" : "false")
								  : flag.error().toString());

			const auto text = runtime.call<std::string>(module, "name");
			check(text.hasValue() && text.value() == "features.py",
				  "call<std::string>(\"name\")",
				  text.hasValue() ? text.value() : text.error().toString());

			// None is a successful call without a value.
			const auto none = runtime.call<std::int64_t>(module, "nothing");
			check(none.isOk() && !none.hasValue(), "a function returning None",
				  "isOk() and no value");

			// A value of another type is reported, not converted.
			const auto wrong = runtime.call<std::int64_t>(module, "name");
			check(wrong.error().code() == mais::ErrorCode::TypeMismatch,
				  "call<std::int64_t>() on a function returning a string",
				  codeName(wrong.error()));

			// A native argument: Python borrows a C++ object of the host.
			services.frame.index  = 3;
			services.frame.clicks = 2;
			const mais::Error native =
				runtime.call(module, "read_context",
							 { scripting::frameArgument(services.frame) });
			check(native.isOk()
					  && services.frame.status == "frame 3, 2 click(s)",
				  "ScriptArgument::native(): the script writes context.status",
				  services.frame.status);

			// What the host bound read-only stays read-only.
			const mais::Error readOnly =
				runtime.call(module, "write_read_only",
							 { scripting::frameArgument(services.frame) });
			check(readOnly.exceptionType() == "AttributeError"
					  && services.frame.clicks == 2,
				  "writing a read-only field from Python",
				  readOnly.exceptionType() + ", clicks is still "
					  + std::to_string(services.frame.clicks));
		}

		/**
		 * @brief What a failed call reports.
		 * @param runtime The running runtime.
		 */
		void tourErrors(mais::ScriptRuntime &runtime)
		{
			// call() reports a missing function; callOptional() treats it as
			// an optional hook.
			const mais::Error missing = runtime.call(module, "on_strat");
			check(missing.code() == mais::ErrorCode::FunctionNotFound,
				  "call() of a missing function", codeName(missing));

			const mais::Error optional =
				runtime.callOptional(module, "on_strat");
			check(optional.isOk(), "callOptional() of a missing function",
				  codeName(optional));

			// A Python exception: code, type, message, frames, traceback.
			const mais::Error raised = runtime.call(module, "fail");
			check(raised.code() == mais::ErrorCode::InvocationFailed
					  && mais::isRecoverable(raised.code()),
				  "a script that raises", codeName(raised) + " (recoverable)");
			check(raised.exceptionType() == "ValueError",
				  "Error::exceptionType()", raised.exceptionType());

			std::string frames;
			for (const mais::Error::Frame &frame: raised.frames()) {
				frames += (frames.empty() ? "" : " > ") + frame.function + ":"
					+ std::to_string(frame.line);
			}
			check(raised.frames().size() >= 2
					  && raised.frames().back().function == "_check",
				  "Error::frames(), outermost first", frames);
			check(raised.hasTraceback(), "Error::toString()",
				  "\n" + raised.toString());

			// The interpreter is still usable after a failure.
			const auto after = runtime.call<std::int64_t>(module, "answer");
			check(after.hasValue(), "a call after a failure", "works");
		}

		/**
		 * @brief Failure policies and the call report.
		 * @param runtime The running runtime.
		 */
		void tourPolicies(mais::ScriptRuntime &runtime)
		{
			// ReportOnce: the first failure of a streak is reported, its
			// identical repeats are counted and suppressed.
			const auto flaky = [&runtime](bool shouldFail) {
				return runtime.call(
					mais::FailurePolicy::ReportOnce, module, "flaky",
					{ mais::ScriptArgument::boolean(shouldFail) });
			};
			const mais::CallReport first  = flaky(true);
			const mais::CallReport second = flaky(true);
			const mais::CallReport third  = flaky(true);
			check(!first.isOk() && !first.suppressed,
				  "ReportOnce, first failure",
				  "reported: " + first.error.message());
			check(second.isOk() && second.suppressed && third.suppressed
					  && third.consecutiveFailures == 3,
				  "ReportOnce, repeated failures",
				  "suppressed, consecutiveFailures = "
					  + std::to_string(third.consecutiveFailures));

			// A successful call ends the streak: the next failure is
			// reported again.
			const mais::CallReport recovered = flaky(false);
			const mais::CallReport again	 = flaky(true);
			check(recovered.isOk() && !recovered.suppressed && !again.isOk()
					  && again.consecutiveFailures == 1,
				  "ReportOnce, failure after a success", "reported again");

			// Stop: the report asks the host to leave its loop.
			const mais::CallReport stop =
				runtime.call(mais::FailurePolicy::Stop, module, "fail");
			check(!stop.isOk() && stop.stopRequested, "Stop",
				  "stopRequested is true");

			// Continue: every failure is reported and nothing is requested.
			const mais::CallReport next =
				runtime.call(mais::FailurePolicy::Continue, module, "fail");
			check(!next.isOk() && !next.stopRequested && !next.suppressed,
				  "Continue", "reported, stopRequested is false");

			// Every report carries the duration of the call.
			const mais::CallReport timed =
				runtime.call(mais::FailurePolicy::Continue, module, "wait",
							 { mais::ScriptArgument::integer(20) });
			const auto milliseconds =
				std::chrono::duration_cast<std::chrono::milliseconds>(
					timed.duration)
					.count();
			check(timed.isOk() && milliseconds >= 20,
				  "CallReport::duration of a 20 ms function",
				  std::to_string(milliseconds) + " ms");
		}

		/**
		 * @brief A command of the `host` module, and the threading rule.
		 * @param runtime The running runtime.
		 * @param services The state the command writes to.
		 */
		void tourHost(mais::ScriptRuntime &runtime,
					  scripting::Services &services)
		{
			// A command is a request: it sets a flag the host loop reads.
			services.quitRequested = false;
			const mais::Error quit = runtime.call(module, "quit");
			check(quit.isOk() && services.quitRequested,
				  "host.request_quit() from Python",
				  "Services::quitRequested is true");
			services.quitRequested = false;

			// Only the thread that called initialize() may call Python.
			// Another thread is refused at once instead of blocking.
			mais::Error fromThread;
			std::thread worker([&runtime, &fromThread]() {
				fromThread = runtime.callOptional(module, "answer");
			});
			worker.join();
			check(fromThread.code() == mais::ErrorCode::WrongThread,
				  "a call from another thread", codeName(fromThread));

			const auto owner = runtime.call<std::int64_t>(module, "answer");
			check(owner.hasValue(), "the owning thread afterwards",
				  "still works");
		}

		/**
		 * @brief Stop Python and start it again in the same process.
		 * @param runtime The running runtime.
		 */
		void tourRestart(mais::ScriptRuntime &runtime)
		{
			const mais::Error stopped = runtime.shutdown();
			check(stopped.isOk() && !runtime.isRunning(), "shutdown()",
				  "Python is stopped");

			const mais::Error late = runtime.call(module, "answer");
			check(late.code() == mais::ErrorCode::NotInitialized,
				  "call() after shutdown()", codeName(late));

			// The queued `host` module and the search paths are kept; the
			// modules have to be loaded again.
			const mais::Error restarted = runtime.initialize();
			const bool forgotten = !runtime.isModuleLoaded(std::string(module));
			const mais::Error reloaded =
				runtime.loadModule(std::string(module));
			const auto again = runtime.call<std::int64_t>(module, "answer");
			check(restarted.isOk() && forgotten && reloaded.isOk()
					  && again.hasValue() && again.value() == 42,
				  "initialize() again, then loadModule()",
				  again.hasValue() ? "answer() returns 42 again"
								   : again.error().toString());

			const mais::Error end = runtime.shutdown();
			check(end.isOk(), "final shutdown()", codeName(end));
		}
	}	 // namespace

	int runScriptingTour(scripting::Services &services)
	{
		// Declared after `services`, which Python borrows.
		mais::ScriptRuntime runtime;

		failures = 0;
		if (tourLifecycle(runtime, services) && tourLoading(runtime)) {
			tourIntrospection(runtime);
			tourValues(runtime, services);
			tourErrors(runtime);
			tourPolicies(runtime);
			tourHost(runtime, services);
			tourRestart(runtime);
		}
		if (runtime.isRunning()) {
			runtime.shutdown();
		}

		std::cout << "[tour] " << (failures == 0 ? "all checks passed" : "")
				  << (failures == 0
						  ? ""
						  : std::to_string(failures) + " check(s) failed")
				  << std::endl;
		return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
	}

}	 // namespace app::desktop
