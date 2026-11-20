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
 * @brief Gallery page 7: app bar and navigation bar.
 *
 * AppBar and NavigationBar.
 */

#include "scenes/gallery/GalleryPages.hpp"

#include <cstddef>
#include <string>

#include <guillaume/theme.hpp>

#include <guillaume/entities/app_bar.hpp>
#include <guillaume/entities/navigation_bar.hpp>

namespace app::scenes::gallery
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;

	void NavigationPage::onEnter(void)
	{
		auto &registry = getComponentRegistry();

		beginPage("7 / 11  App bar and navigation bar");

		// App bar.
		{
			using AppBar = entities::AppBar;
			auto row	 = beginRow();
			AppBar::Builder builder(registry, *row);
			auto appBar =
				builder.withTitle("Gallery")
					.withNavigationIcon("menu")
					.withOnNavigation(logger("App bar navigation"), "Menu")
					.addAction("search", logger("App bar: search"), "Search")
					.addAction("more_vert", logger("App bar: more"), "More")
					.withFixedWidth(480.0f)
					.makeAppBar(row, AppBar::Variant::Small);
			endRow(row, "AppBar", { appBar });
		}

		// Navigation bar.
		{
			using Bar = entities::NavigationBar;
			auto row  = beginRow();
			Bar::Builder builder(registry, *row);
			auto bar = builder.makeNavigationBar(row, Bar::Variant::WithLabels,
												 { { "home", "Home" },
												   { "explore", "Explore" },
												   { "person", "Profile" } });
			bar->setOnIndexChange([](std::size_t index) {
				logger("Navigation bar: " + std::to_string(index))();
			});
			bar->setBadge(1, "3");
			endRow(row, "NavigationBar", { bar });
		}

		endPage<FeedbackPage, DrawerPage>();
	}

}	 // namespace app::scenes::gallery
