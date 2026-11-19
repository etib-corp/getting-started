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

#include <chrono>

#include "AppContext.hpp"

/**
 * @file
 * @brief What every platform host does around one frame.
 *
 * A host loop is: FrameClock::tick(), then the script hook or
 * writeDefaultStatus(), then presentStatus(), then one guillaume frame.
 */

namespace app
{
	/**
	 * @brief Measures the frame times a host hands to the scripts and the UI.
	 */
	class FrameClock
	{
		public:
		/**
		 * @brief Start the clock: elapsed time counts from here.
		 */
		FrameClock(void);

		/**
		 * @brief Start a frame.
		 *
		 * Writes the time since the previous call and since construction,
		 * and advances the frame index on every call but the first.
		 * @param frame The frame context to update.
		 */
		void tick(scripting::FrameContext &frame);

		private:
		using Clock = std::chrono::steady_clock;

		Clock::time_point _start;		///< When the clock was created
		Clock::time_point _previous;	///< When the previous frame started
		bool _first = true;				///< True until the first tick()
	};

	/**
	 * @brief Write the status line when no script does it.
	 *
	 * Used by a host whose script is not running (on a headset, when Python
	 * or the script failed to start), so the sample still shows a line.
	 * @param frame The frame context; `status` is written.
	 */
	void writeDefaultStatus(scripting::FrameContext &frame);

	/**
	 * @brief Show the status line in the active scene when it changed.
	 * @param context The application context.
	 */
	void presentStatus(Context &context);

}	 // namespace app
