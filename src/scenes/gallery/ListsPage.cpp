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
 * @brief Gallery page 5: dividers, badges and lists.
 *
 * Divider, Badge and List (subheader, divider, accordion).
 */

#include "scenes/gallery/GalleryPages.hpp"

#include <guillaume/theme.hpp>

#include <guillaume/entities/badge.hpp>
#include <guillaume/entities/divider.hpp>
#include <guillaume/entities/list.hpp>

namespace app::scenes::gallery
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;

	void ListsPage::onEnter(void)
	{
		auto &registry = getComponentRegistry();

		beginPage("5 / 11  Dividers, badges and lists");

		// Dividers and badges.
		{
			using Divider = entities::Divider;
			using Badge	  = entities::Badge;
			auto row	  = beginRow();
			Divider::Builder dividerBuilder(registry, *row);
			Badge::Builder badgeBuilder(registry, *row);
			Entities widgets;
			widgets.push_back(dividerBuilder.makeDivider(
				row, 120.0f, Divider::Variant::FullWidth));
			widgets.push_back(dividerBuilder.makeDivider(
				row, 120.0f, Divider::Variant::Inset));
			widgets.push_back(
				badgeBuilder.makeBadge(row, "", Badge::Variant::Dot));
			widgets.push_back(
				badgeBuilder.makeBadge(row, "3", Badge::Variant::Small));
			widgets.push_back(
				badgeBuilder.makeBadge(row, "999+", Badge::Variant::Large));
			endRow(row, "Divider, Badge", widgets);
		}

		// List with a subheader, rows, a divider and an accordion.
		{
			using List = entities::List;
			auto row   = beginRow();
			List::Builder builder(registry, *row);

			List::Item inbox;
			inbox.headline	  = "Inbox";
			inbox.supporting  = "3 unread";
			inbox.leadingIcon = "inbox";
			inbox.onPress	  = logger("List: inbox");
			List::Item sent;
			sent.headline	 = "Sent";
			sent.leadingIcon = "send";
			sent.onPress	 = logger("List: sent");
			List::Item folders;
			folders.headline	= "Folders (click to expand)";
			folders.leadingIcon = "folder";
			List::Item work;
			work.headline	  = "Work";
			work.onPress	  = logger("List: work");
			List::Item travel = work;
			travel.headline	  = "Travel (disabled)";
			travel.disabled	  = true;
			folders.children  = { work, travel };

			auto list = builder.makeList(row, List::Variant::TwoLine,
										 { List::subheader("Mail"), inbox, sent,
										   List::divider(), folders });
			endRow(row, "List", { list });
		}

		endPage<ContainersPage, FeedbackPage>();
	}

}	 // namespace app::scenes::gallery
