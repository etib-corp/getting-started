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

/**
 * @file
 * @brief The application state the hosts fill and the scripts can see.
 *
 * Plain structures with no dependency on Python, mais or an engine. Both
 * hosts hand them to the Python hooks; a host that runs without a script
 * fills them itself.
 */

namespace app::scripting
{
	/**
	 * @brief Start-up settings a script may propose in `configure(settings)`.
	 *
	 * The host validates the proposal with validate() before the window is
	 * created; an invalid proposal is replaced by the defaults. A headset has
	 * no window and paces and samples its frames itself: the Android host
	 * validates the proposal and does not use it.
	 */
	struct Settings {
		std::string windowTitle = "Getting started";	///< Window title
		int width				= 1280;	   ///< Window width, in pixels
		int height				= 800;	   ///< Window height, in pixels

		/// Frame rate limit of the engine; 0 leaves the pacing to the
		/// display (vsync).
		double targetFps = 0.0;

		/// Samples per pixel of the engine's anti-aliasing: 1 (off), 2 or 4.
		int msaaSamples = 1;

		/// Whether the engine moves the view with W/A/S/D, Q/E, the arrow
		/// keys and a right-button drag.
		bool cameraControls = true;
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
	 * @brief Check a settings proposal.
	 * @param settings The proposal.
	 * @return One message per problem; empty when the proposal is usable.
	 */
	inline std::vector<std::string> validate(const Settings &settings)
	{
		std::vector<std::string> problems;

		if (settings.windowTitle.empty()) {
			problems.emplace_back("window_title must not be empty");
		} else if (settings.windowTitle.size() > 64) {
			problems.emplace_back("window_title must be at most 64 characters");
		}
		if (settings.width < 320 || settings.width > 7680) {
			problems.emplace_back("width must be between 320 and 7680 (got "
								  + std::to_string(settings.width) + ")");
		}
		if (settings.height < 240 || settings.height > 4320) {
			problems.emplace_back("height must be between 240 and 4320 (got "
								  + std::to_string(settings.height) + ")");
		}
		if (settings.targetFps < 0.0 || settings.targetFps > 240.0) {
			problems.emplace_back("target_fps must be between 0 and 240 (got "
								  + std::to_string(settings.targetFps) + ")");
		}
		if (settings.msaaSamples != 1 && settings.msaaSamples != 2
			&& settings.msaaSamples != 4) {
			problems.emplace_back("msaa_samples must be 1, 2 or 4 (got "
								  + std::to_string(settings.msaaSamples) + ")");
		}
		return problems;
	}

}	 // namespace app::scripting
