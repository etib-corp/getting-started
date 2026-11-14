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

#include "Frame.hpp"

#include <string>

namespace app
{
	FrameClock::FrameClock(void)
		: _start(Clock::now())
		, _previous(_start)
	{
	}

	void FrameClock::tick(scripting::FrameContext &frame)
	{
		const Clock::time_point now = Clock::now();

		frame.deltaSeconds =
			std::chrono::duration<double>(now - _previous).count();
		frame.elapsedSeconds =
			std::chrono::duration<double>(now - _start).count();
		_previous = now;

		if (_first) {
			_first = false;
		} else {
			++frame.index;
		}
	}

	void writeDefaultStatus(scripting::FrameContext &frame)
	{
		const auto seconds = static_cast<long long>(frame.elapsedSeconds);

		frame.status = std::to_string(frame.clicks) + " click(s), running for "
			+ std::to_string(seconds) + " s";
	}

	void presentStatus(Context &context)
	{
		const std::string &status = context.scripting.frame.status;

		if (context.showStatus && status != context.shownStatus) {
			context.shownStatus = status;
			context.showStatus(status);
		}
	}

}	 // namespace app
