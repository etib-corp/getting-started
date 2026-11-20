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
 * @brief Gallery page 8: navigation drawers.
 *
 * NavigationDrawer, standard and collapsed.
 */

#include "scenes/gallery/GalleryPages.hpp"

#include <cstddef>
#include <string>

#include <guillaume/theme.hpp>

#include <guillaume/entities/navigation_drawer.hpp>

namespace app::scenes::gallery
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;

	void DrawerPage::onEnter(void)
	{
		auto &registry = getComponentRegistry();

		beginPage("8 / 11  Navigation drawers");

		// Navigation drawer, standard and collapsed.
		{
			using Drawer = entities::NavigationDrawer;
			auto row	 = beginRow();
			Drawer::Builder builder(registry, *row);
			Entities widgets;
			auto drawer =
				builder.makeNavigationDrawer(row, Drawer::Variant::Standard,
											 { { "inbox", "Inbox" },
											   { "send", "Sent" },
											   { "delete", "Trash" } });
			drawer->setOnIndexChange([](std::size_t index) {
				logger("Drawer: " + std::to_string(index))();
			});
			widgets.push_back(drawer);
			widgets.push_back(builder.makeNavigationDrawer(
				row, Drawer::Variant::Collapsed,
				{ { "inbox", "Inbox" }, { "send", "Sent" } }));

			endRow(row, "NavigationDrawer", widgets);
		}

		endPage<NavigationPage, MeshPage>();
	}

}	 // namespace app::scenes::gallery
