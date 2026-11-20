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
#include <memory>
#include <string>
#include <vector>

#include <guillaume/scene.hpp>
#include <guillaume/theme.hpp>

#include <guillaume/entities/button.hpp>
#include <guillaume/entities/text.hpp>
#include <guillaume/entities/view.hpp>

#include <utility/graphic/color.hpp>
#include <utility/graphic/pose.hpp>

#include "scenes/HomeScene.hpp"

/**
 * @file
 * @brief The widget gallery: one scene per family of guillaume widgets.
 *
 * Each page is a scene with one source file under src/scenes/gallery/, so
 * the code that creates a widget is next to the widget's name:
 *
 * | Page | File | Widgets |
 * | --- | --- | --- |
 * | 1 | ButtonsPage.cpp | Button, IconButton, FABs, FAB menu, SegmentedButton |
 * | 2 | SelectionPage.cpp | Checkbox, RadioButton, RadioGroup, Switch, Chip |
 * | 3 | TextPage.cpp | Text, Icon, Image, TextField, Search |
 * | 4 | ContainersPage.cpp | Surface, Card |
 * | 5 | ListsPage.cpp | Divider, Badge, List |
 * | 6 | FeedbackPage.cpp | ProgressIndicator, LoadingIndicator, Snackbar,
 * Dialog, Menu, Tooltip | | 7 | NavigationPage.cpp | AppBar, NavigationBar | |
 * 8 | DrawerPage.cpp | NavigationDrawer | | 9 | MeshPage.cpp | Mesh | | 10 |
 * ThemePage.cpp | colour schemes, theme tokens, accessibility | | 11 |
 * StoragePage.cpp | LocalStorage, SessionStorage |
 *
 * Pages 1 to 9 follow the widget gallery of guillaume
 * (examples/evan/main.cpp in its repository). Every interaction prints a
 * `[gallery]` line on standard output, so a click that has no visible effect
 * can still be checked.
 */

namespace app::scenes::gallery
{
	/// The widgets of a row or of a page.
	using Entities = std::vector<std::shared_ptr<guillaume::ecs::Entity>>;

	/**
	 * @brief Make a handler that logs an interaction on standard output.
	 * @param message What happened.
	 * @return A callback that prints `[gallery] <message>`.
	 */
	std::function<void(void)> logger(const std::string &message);

	/**
	 * @brief Color of a role in the active theme.
	 * @param role The scheme role.
	 * @return The color.
	 */
	utility::graphic::Color32Bit color(guillaume::SchemeColorRole role);

	/**
	 * @brief Base of the gallery pages.
	 *
	 * A page is one root View: a column holding a title, rows of widgets and
	 * a navigation row. Every widget is owned by the row it sits in, so the
	 * scene has a single root to place in front of the camera. Leaving the
	 * page removes everything it built, so entering it again starts fresh.
	 */
	class Page: public guillaume::Scene
	{
		public:
		using guillaume::Scene::Scene;

		/**
		 * @brief Remove the page's entities and their graphics.
		 */
		void onExit(void) override;

		protected:
		/**
		 * @brief Start a page: create the root column and its title.
		 * @param title The page title.
		 */
		void beginPage(const std::string &title);

		/**
		 * @brief Create an empty row owned by the page.
		 * @return The row. Build widgets with it as owner, then call
		 * endRow().
		 */
		std::shared_ptr<guillaume::entities::View> beginRow(void);

		/**
		 * @brief Lay out a row: a caption followed by its widgets.
		 * @param row The row from beginRow().
		 * @param caption Short text shown first in the row.
		 * @param widgets The widgets, in order.
		 */
		void endRow(const std::shared_ptr<guillaume::entities::View> &row,
					const std::string &caption, const Entities &widgets);

		/**
		 * @brief Add the Previous / Home / Next row and lay out the page.
		 * @tparam Previous The scene shown by Previous.
		 * @tparam Next The scene shown by Next.
		 */
		template<typename Previous, typename Next> void endPage(void)
		{
			using Button = guillaume::entities::Button;

			auto row = beginRow();
			Button::Builder buttonBuilder(getComponentRegistry(), *row);
			auto previous = buttonBuilder.makeIconButton(
				row, "Previous", "arrow_back",
				guillaume::components::Glyph::Style::Outlined,
				[this]() {
					goToScene<Previous>();
				},
				Button::Color::Tonal, Button::Shape::Round, Button::Size::Small,
				false);
			auto home = buttonBuilder.makeIconButton(
				row, "Home", "home",
				guillaume::components::Glyph::Style::Outlined,
				[this]() {
					goToScene<HomeScene>();
				},
				Button::Color::Outlined, Button::Shape::Round,
				Button::Size::Small, false);
			auto next = buttonBuilder.makeIconButton(
				row, "Next", "arrow_forward",
				guillaume::components::Glyph::Style::Outlined,
				[this]() {
					goToScene<Next>();
				},
				Button::Color::Filled, Button::Shape::Round,
				Button::Size::Small, false);
			endRow(row, "Pages", { previous, home, next });

			_page->setEntities(_rows);
		}

		private:
		std::shared_ptr<guillaume::entities::View> _page;	 ///< Root column
		Entities _rows;										 ///< Title and rows
	};

	/// Page 1: Button, IconButton, FABs, FAB menu, SegmentedButton.
	class ButtonsPage: public Page
	{
		public:
		using Page::Page;
		void onEnter(void) override;
	};

	/// Page 2: Checkbox, RadioButton, RadioGroup, Switch, Chip.
	class SelectionPage: public Page
	{
		public:
		using Page::Page;
		void onEnter(void) override;
	};

	/// Page 3: Text, Icon, Image, TextField, Search.
	class TextPage: public Page
	{
		public:
		using Page::Page;
		void onEnter(void) override;
	};

	/// Page 4: Surface, Card.
	class ContainersPage: public Page
	{
		public:
		using Page::Page;
		void onEnter(void) override;
	};

	/// Page 5: Divider, Badge, List.
	class ListsPage: public Page
	{
		public:
		using Page::Page;
		void onEnter(void) override;
	};

	/// Page 6: indicators, Snackbar, Dialog, Menu, Tooltip.
	class FeedbackPage: public Page
	{
		public:
		using Page::Page;
		void onEnter(void) override;
	};

	/// Page 7: AppBar, NavigationBar.
	class NavigationPage: public Page
	{
		public:
		using Page::Page;
		void onEnter(void) override;
	};

	/// Page 8: NavigationDrawer.
	class DrawerPage: public Page
	{
		public:
		using Page::Page;
		void onEnter(void) override;
	};

	/// Page 9: Mesh.
	class MeshPage: public Page
	{
		public:
		using Page::Page;
		void onEnter(void) override;
	};

	/// Page 10: colour schemes, theme tokens, accessibility.
	class ThemePage: public Page
	{
		public:
		using Page::Page;
		void onEnter(void) override;
	};

	/// Page 11: LocalStorage and SessionStorage.
	class StoragePage: public Page
	{
		public:
		using Page::Page;
		void onEnter(void) override;
	};

}	 // namespace app::scenes::gallery
