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

#include <mais/ScriptRuntime.hpp>

#include "scripting/Services.hpp"

/**
 * @file
 * @brief The Python side of the application state.
 *
 * Scripts never receive the engine or a widget. They receive the structures
 * of Services.hpp, which the host owns, fills before a hook and reads back
 * after it. To expose something new to Python, add a field or a command
 * there and bind it in Scripting.cpp.
 *
 * This header includes no pybind11 header, so the host does not depend on it.
 * It is only built on platforms where scripting is enabled.
 */

namespace app::scripting
{
	/**
	 * @brief Queue the `host` Python module.
	 *
	 * Call once, before mais::ScriptRuntime::initialize().
	 * @param runtime The runtime the module is registered in.
	 * @param services The host state the module gives access to.
	 * @return An error when the module could not be queued.
	 */
	mais::Error registerBindings(mais::ScriptRuntime &runtime,
								 Services &services);

	/**
	 * @brief Wrap the settings as the argument of `configure(settings)`.
	 * @param settings The proposal the script writes to.
	 * @return The argument to pass to ScriptRuntime::call().
	 */
	mais::ScriptArgument settingsArgument(Settings &settings);

	/**
	 * @brief Wrap the frame context as the argument of the frame hooks.
	 * @param context The context the script reads.
	 * @return The argument to pass to ScriptRuntime::call().
	 */
	mais::ScriptArgument frameArgument(FrameContext &context);

}	 // namespace app::scripting
