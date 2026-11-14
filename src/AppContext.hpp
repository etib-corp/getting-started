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

#include <functional>
#include <string>

#include "scripting/Services.hpp"

/**
 * @file
 * @brief State shared between the host loop and the scenes.
 *
 * guillaume creates the scenes itself, so a scene cannot receive constructor
 * arguments from the host. Both sides reach the shared state through context()
 * instead.
 */

namespace app
{
	/**
	 * @brief What the host loop and the scenes exchange.
	 */
	struct Context {
		/// Settings, per-frame values and requests. Exposed to the Python
		/// scripts on platforms where scripting is enabled.
		scripting::Services scripting;

		/// Set by the active scene: shows a line of text in the window. Empty
		/// when no scene displays the status.
		std::function<void(const std::string &)> showStatus;

		/// The status currently shown, so it is only pushed when it changes.
		/// A scene clears it when it sets showStatus.
		std::string shownStatus;
	};

	/**
	 * @brief Get the application context.
	 *
	 * The context lives until the program exits, so it outlives the script
	 * runtime and the scenes.
	 * @return The context.
	 */
	Context &context(void);

}	 // namespace app
