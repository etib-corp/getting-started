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

#include <memory>

#include <guillaume/scene.hpp>

#include <guillaume/entities/text.hpp>
#include <guillaume/entities/view.hpp>

namespace app::scenes
{
	/**
	 * @brief The scene shown at start-up.
	 *
	 * A card with a title, a line of text written by the Python script and a
	 * button. Copy this class to add a scene, then list the new class in the
	 * guillaume::Application template arguments in main.cpp and switch to it
	 * with goToScene<OtherScene>().
	 */
	class HomeScene: public guillaume::Scene
	{
		public:
		using guillaume::Scene::Scene;

		/**
		 * @brief Build the widgets.
		 */
		void onEnter(void) override;

		/**
		 * @brief Remove the widgets.
		 */
		void onExit(void) override;

		private:
		std::shared_ptr<guillaume::entities::View> _page;	 ///< Root column
		std::shared_ptr<guillaume::entities::Text>
			_status;	///< Line written by the script
	};

}	 // namespace app::scenes
