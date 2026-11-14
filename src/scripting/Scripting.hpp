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

#include <cstdint>
#include <string>
#include <vector>

#include <mais/ScriptRuntime.hpp>

/**
 * @file
 * @brief What the Python scripts can see of the application.
 *
 * Scripts never receive the engine or a widget. They receive the small
 * structures below, which the host (main.cpp) owns, fills before a hook and
 * reads back after it. To expose something new to Python, add a field or a
 * command here and bind it in Scripting.cpp.
 *
 * This header includes no pybind11 header, so the rest of the application
 * does not depend on it.
 */

namespace app::scripting
{
	/**
	 * @brief Start-up settings a script may propose in `configure(settings)`.
	 *
	 * The host validates the proposal with validate() before the window is
	 * created; an invalid proposal is replaced by the defaults.
	 */
	struct Settings {
		std::string windowTitle = "Getting started";	///< Window title
		int width				= 1280;	   ///< Window width, in pixels
		int height				= 800;	   ///< Window height, in pixels
	};

	/**
	 * @brief What a script receives in `on_start`, `on_update` and
	 * `on_shutdown`.
	 *
	 * The host rewrites the read-only fields before each call. `status` is the
	 * one field a script writes: the host shows it in the window.
	 */
	struct FrameContext {
		std::uint64_t index	  = 0;		///< Frame number, from 0 (read-only)
		double deltaSeconds	  = 0.0;	///< Time since last frame (read-only)
		double elapsedSeconds = 0.0;	///< Time since start (read-only)
		std::uint64_t clicks  = 0;		///< Clicks on the button (read-only)
		std::string status;				///< Text shown in the window (writable)
	};

	/**
	 * @brief Everything the bindings reach.
	 *
	 * Owned by the host. It must be created before the mais::ScriptRuntime and
	 * destroyed after it, because Python keeps references to these objects.
	 */
	struct Services {
		Settings settings;			   ///< The proposal handed to `configure`
		FrameContext frame;			   ///< The context handed to the hooks
		bool quitRequested = false;	   ///< Set by `host.request_quit()`
	};

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

	/**
	 * @brief Check a settings proposal.
	 * @param settings The proposal.
	 * @return One message per problem; empty when the proposal is usable.
	 */
	std::vector<std::string> validate(const Settings &settings);

}	 // namespace app::scripting
