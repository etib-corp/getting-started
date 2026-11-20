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

#pragma once

#include "scripting/Services.hpp"

/**
 * @file
 * @brief A tour of the maïs runtime, run with `--scripting-tour`.
 */

namespace app::desktop
{
	/**
	 * @brief Exercise the maïs runtime feature by feature, without a window.
	 *
	 * Starts its own mais::ScriptRuntime, loads scripts/features.py and
	 * prints one `[tour]` line per check: lifecycle, search paths, module
	 * loading, introspection, arguments, typed return values, error reports,
	 * failure policies, call duration, host commands, thread affinity and
	 * restart. Must run before any other runtime exists in the process.
	 * @param services The state the `host` module gives access to.
	 * @return EXIT_SUCCESS when every check passed.
	 */
	int runScriptingTour(scripting::Services &services);

}	 // namespace app::desktop
