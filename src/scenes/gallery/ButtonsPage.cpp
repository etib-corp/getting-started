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
 * @brief Gallery page 1: buttons.
 *
 * Button, IconButton, floating action buttons, FAB menu and SegmentedButton.
 */

#include "scenes/gallery/GalleryPages.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include <guillaume/theme.hpp>

#include <guillaume/entities/button.hpp>
#include <guillaume/entities/extended_floating_action_button.hpp>
#include <guillaume/entities/floating_action_button.hpp>
#include <guillaume/entities/floating_action_button_menu.hpp>
#include <guillaume/entities/icon_button.hpp>
#include <guillaume/entities/segmented_button.hpp>

namespace app::scenes::gallery
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;

	void ButtonsPage::onEnter(void)
	{
		using Button   = entities::Button;
		auto &registry = getComponentRegistry();

		beginPage("1 / 11  Buttons");

		// Common buttons: one per color style, plus a toggle and a disabled.
		{
			auto row = beginRow();
			Button::Builder builder(registry, *row);
			Entities widgets;
			const std::vector<std::pair<std::string, Button::Color>> styles {
				{ "Elevated", Button::Color::Elevated },
				{ "Filled", Button::Color::Filled },
				{ "Tonal", Button::Color::Tonal },
				{ "Outlined", Button::Color::Outlined },
				{ "Text", Button::Color::Text },
			};
			for (const auto &[label, style]: styles) {
				widgets.push_back(builder.makeButton(
					row, label, logger(label + " button clicked"), style,
					Button::Shape::Round, Button::Size::Small, false));
			}
			auto withIcon = builder.makeIconButton(
				row, "With icon", "add",
				guillaume::components::Glyph::Style::Outlined,
				logger("Icon + label button clicked"), Button::Color::Filled,
				Button::Shape::Square, Button::Size::Small, false);
			widgets.push_back(withIcon);
			auto disabled = builder.makeButton(
				row, "Disabled", logger("BUG: a disabled button was clicked"),
				Button::Color::Filled, Button::Shape::Round,
				Button::Size::Small, false);
			disabled->setDisabled(true);
			widgets.push_back(disabled);
			endRow(row, "Button", widgets);
		}

		// Icon buttons: the four variants, and a toggle.
		{
			using IconButton = entities::IconButton;
			auto row		 = beginRow();
			IconButton::Builder builder(registry, *row);
			Entities widgets;
			const std::vector<std::pair<std::string, IconButton::Variant>>
				variants {
					{ "Standard", IconButton::Variant::Standard },
					{ "Filled", IconButton::Variant::Filled },
					{ "Tonal", IconButton::Variant::Tonal },
					{ "Outlined", IconButton::Variant::Outlined },
				};
			for (const auto &[label, variant]: variants) {
				widgets.push_back(builder.makeIconButton(
					row, "favorite", logger(label + " icon button clicked"),
					variant, IconButton::Size::Medium));
			}
			auto toggle = builder.makeIconButton(
				row, "bookmark", logger("Toggle icon button"),
				IconButton::Variant::Filled, IconButton::Size::Medium);
			toggle->setIsToggle(true);
			toggle->setSelectedIconGlyphName("bookmark_added");
			widgets.push_back(toggle);
			endRow(row, "IconButton", widgets);
		}

		// Floating action buttons: sizes and variants, extended, menu.
		{
			using Fab = entities::FloatingActionButton;
			auto row  = beginRow();
			Fab::Builder builder(registry, *row);
			Entities widgets;
			widgets.push_back(builder.makeFloatingActionButton(
				row, "edit", logger("Small FAB clicked"), Fab::Variant::Primary,
				Fab::Size::Small));
			widgets.push_back(builder.makeFloatingActionButton(
				row, "edit", logger("Medium FAB clicked"),
				Fab::Variant::Secondary, Fab::Size::Medium));
			widgets.push_back(builder.makeFloatingActionButton(
				row, "edit", logger("Large FAB clicked"),
				Fab::Variant::Tertiary, Fab::Size::Large));
			widgets.push_back(builder.makeFloatingActionButton(
				row, "edit", logger("Surface FAB clicked"),
				Fab::Variant::Surface, Fab::Size::Medium));

			entities::ExtendedFloatingActionButton::Builder extendedBuilder(
				registry, *row);
			auto extended = extendedBuilder.makeExtendedFloatingActionButton(
				row, "Compose", {});
			// Each click collapses or expands the label.
			auto *extendedPointer = extended.get();
			extended->setOnClick([extendedPointer]() {
				extendedPointer->setExtended(!extendedPointer->isExtended());
				logger("Extended FAB toggled")();
			});
			widgets.push_back(extended);

			entities::FloatingActionButtonMenu::Builder menuBuilder(registry,
																	*row);
			widgets.push_back(
				menuBuilder.addAction("mail", "Mail", logger("FAB menu: mail"))
					.addAction("chat", "Chat", logger("FAB menu: chat"))
					.addAction("call", "Call", logger("FAB menu: call"))
					.makeFloatingActionButtonMenu(row, "add"));
			endRow(row, "FAB", widgets);
		}

		// Segmented buttons: single and multi selection.
		{
			using Segmented = entities::SegmentedButton;
			auto row		= beginRow();
			Segmented::Builder builder(registry, *row);
			Entities widgets;
			widgets.push_back(builder.makeSegmentedButton(
				row, { "Day", "Week", "Month" },
				Segmented::SelectionMode::Single, [](std::size_t index) {
					logger("Segment selected: " + std::to_string(index))();
				}));
			auto multi = builder.makeSegmentedButton(
				row, { "Bold", "Italic", "Underline" },
				Segmented::SelectionMode::Multi, [](std::size_t index) {
					logger("Segment toggled: " + std::to_string(index))();
				});
			multi->setSegmentDisabled(2, true);
			widgets.push_back(multi);
			endRow(row, "SegmentedButton", widgets);
		}

		endPage<scenes::HomeScene, SelectionPage>();
	}

}	 // namespace app::scenes::gallery
