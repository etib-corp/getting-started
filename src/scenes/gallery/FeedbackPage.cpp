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
 * @brief Gallery page 6: feedback.
 *
 * ProgressIndicator, LoadingIndicator, Snackbar, Dialog, Menu and Tooltip.
 */

#include "scenes/gallery/GalleryPages.hpp"

#include <guillaume/theme.hpp>

#include <guillaume/entities/button.hpp>
#include <guillaume/entities/dialog.hpp>
#include <guillaume/entities/loading_indicator.hpp>
#include <guillaume/entities/menu.hpp>
#include <guillaume/entities/progress_indicator.hpp>
#include <guillaume/entities/snackbar.hpp>
#include <guillaume/entities/tooltip.hpp>

namespace app::scenes::gallery
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;

	void FeedbackPage::onEnter(void)
	{
		auto &registry = getComponentRegistry();

		beginPage("6 / 11  Feedback");

		// Progress and loading indicators.
		{
			using Progress = entities::ProgressIndicator;
			using Loading  = entities::LoadingIndicator;
			auto row	   = beginRow();
			Progress::Builder progressBuilder(registry, *row);
			Loading::Builder loadingBuilder(registry, *row);
			Entities widgets;
			widgets.push_back(progressBuilder.makeProgressIndicator(
				row, Progress::Variant::LinearDeterminate, 0.6f));
			widgets.push_back(progressBuilder.makeProgressIndicator(
				row, Progress::Variant::LinearIndeterminate, 0.0f));
			widgets.push_back(progressBuilder.makeProgressIndicator(
				row, Progress::Variant::CircularDeterminate, 0.3f));
			widgets.push_back(progressBuilder.makeProgressIndicator(
				row, Progress::Variant::CircularIndeterminate, 0.0f));
			widgets.push_back(loadingBuilder.makeLoadingIndicator(
				row, Loading::Variant::Circular));
			widgets.push_back(loadingBuilder.makeLoadingIndicator(
				row, Loading::Variant::Linear));
			endRow(row, "Progress, Loading", widgets);
		}

		// Overlays, each opened by a button. The snackbar sits in the row;
		// the dialog, the menu and the tooltip place themselves.
		{
			using Button = entities::Button;
			auto row	 = beginRow();
			Button::Builder buttonBuilder(registry, *row);
			Entities widgets;

			entities::Snackbar::Builder snackbarBuilder(registry, *row);
			auto snackbar = snackbarBuilder.makeSnackbar(
				row, entities::Snackbar::Variant::WithAction, "Message sent",
				"Undo");
			snackbar->setOnAction(logger("Snackbar action"));
			snackbar->setOnDismiss(logger("Snackbar dismissed"));
			snackbar->setVisible(false);
			auto *snackbarPointer = snackbar.get();
			widgets.push_back(buttonBuilder.makeButton(
				row, "Snackbar",
				[snackbarPointer]() {
					snackbarPointer->show();
				},
				Button::Color::Tonal, Button::Shape::Round, Button::Size::Small,
				false));

			entities::Dialog::Builder dialogBuilder(registry, *row);
			auto dialog = dialogBuilder.makeDialog(
				row, entities::Dialog::Variant::Alert, "Discard draft?",
				"This cannot be undone.", { "Cancel", "Discard" });
			dialog->setOnDismiss(logger("Dialog dismissed"));
			auto *dialogPointer = dialog.get();
			widgets.push_back(buttonBuilder.makeButton(
				row, "Dialog",
				[dialogPointer]() {
					dialogPointer->show();
				},
				Button::Color::Tonal, Button::Shape::Round, Button::Size::Small,
				false));

			auto menuTrigger = buttonBuilder.makeButton(
				row, "Menu", {}, Button::Color::Tonal, Button::Shape::Round,
				Button::Size::Small, false);
			entities::Menu::Item cut;
			cut.label		= "Cut";
			cut.leadingIcon = "content_cut";
			cut.onPress		= logger("Menu: cut");
			entities::Menu::Item copy;
			copy.label		 = "Copy";
			copy.leadingIcon = "content_copy";
			copy.onPress	 = logger("Menu: copy");
			entities::Menu::Item paste;
			paste.label		  = "Paste (disabled)";
			paste.leadingIcon = "content_paste";
			paste.disabled	  = true;
			entities::Menu::Builder menuBuilder(registry, *row);
			auto menu = menuBuilder.makeMenu(
				row, entities::Menu::Variant::Dropdown, { cut, copy, paste });
			menu->setAnchor(menuTrigger->getIdentifier());
			auto *menuPointer = menu.get();
			menuTrigger->setOnClick([menuPointer]() {
				menuPointer->show();
			});
			widgets.push_back(menuTrigger);

			auto tooltipAnchor = buttonBuilder.makeButton(
				row, "Hover me", logger("Tooltip anchor clicked"),
				Button::Color::Outlined, Button::Shape::Round,
				Button::Size::Small, false);
			entities::Tooltip::Builder tooltipBuilder(registry, *row);
			auto tooltip = tooltipBuilder.makeTooltip(
				row, entities::Tooltip::Variant::Plain, "A plain tooltip");
			tooltip->setAnchor(tooltipAnchor);
			widgets.push_back(tooltipAnchor);

			widgets.push_back(snackbar);
			endRow(row, "Overlays", widgets);
		}

		endPage<ListsPage, NavigationPage>();
	}

}	 // namespace app::scenes::gallery
