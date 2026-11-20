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
 * @brief Gallery page 10: theme and accessibility.
 *
 * Colour schemes (light, dark, high contrast), a theme token changed at run
 * time (the roundness of the shape scale) and the accessibility description
 * of a widget.
 */

#include "scenes/gallery/GalleryPages.hpp"

#include <string>
#include <utility>
#include <vector>

#include <guillaume/components/accessibility.hpp>

#include <guillaume/entities/checkbox.hpp>
#include <guillaume/entities/surface.hpp>
#include <guillaume/entities/switch.hpp>

namespace app::scenes::gallery
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;

	void ThemePage::onEnter(void)
	{
		using Button   = entities::Button;
		auto &registry = getComponentRegistry();

		beginPage("10 / 11  Theme and accessibility");

		// Colour schemes. Widgets read their colours when they are built, so
		// the page is entered again after the switch: goToScene<ThemePage>()
		// runs onExit() then onEnter() with the new scheme. The scheme is
		// process-wide: the other pages and the home scene follow it.
		{
			auto row = beginRow();
			Button::Builder builder(registry, *row);
			Entities widgets;
			const std::vector<
				std::pair<std::string, guillaume::ThemeSchemeRole>>
				schemes {
					{ "Light", guillaume::ThemeSchemeRole::Light },
					{ "Dark", guillaume::ThemeSchemeRole::Dark },
					{ "Light high contrast",
					  guillaume::ThemeSchemeRole::LightHighContrast },
					{ "Dark high contrast",
					  guillaume::ThemeSchemeRole::DarkHighContrast },
				};
			for (const auto &[label, scheme]: schemes) {
				const std::string name					= label;
				const guillaume::ThemeSchemeRole target = scheme;
				widgets.push_back(builder.makeButton(
					row, label,
					[this, name, target]() {
						guillaume::setActiveScheme(target);
						logger("Scheme: " + name)();
						goToScene<ThemePage>();
					},
					Button::Color::Tonal, Button::Shape::Round,
					Button::Size::Small, false));
			}
			endRow(row, "Scheme", widgets);
		}

		// A design token: the roundness of the shape scale. The theme is
		// copied, changed and activated; every widget that reads a shape
		// role follows (the surfaces of the next row).
		{
			auto row = beginRow();
			Button::Builder builder(registry, *row);
			Entities widgets;
			const std::vector<std::pair<std::string, float>> values {
				{ "Square (0)", 0.0f },
				{ "Default (4)", 4.0f },
				{ "Round (8)", 8.0f },
			};
			for (const auto &[label, roundness]: values) {
				const std::string name = label;
				const float value	   = roundness;
				widgets.push_back(builder.makeButton(
					row, label,
					[this, name, value]() {
						guillaume::Theme custom = guillaume::getActiveTheme();
						custom.getShapes().setRoundness(value);
						guillaume::setActiveTheme(custom);
						logger("Roundness: " + name)();
						goToScene<ThemePage>();
					},
					Button::Color::Outlined, Button::Shape::Round,
					Button::Size::Small, false));
			}
			endRow(row, "Roundness", widgets);
		}

		// What the two rows above change: colour roles and shape roles.
		{
			using Surface = entities::Surface;
			auto row	  = beginRow();
			Surface::Builder surfaceBuilder(registry, *row);
			Entities widgets;
			for (const auto shape:
				 { theme::ShapeRole::Small, theme::ShapeRole::Medium,
				   theme::ShapeRole::Large }) {
				auto surface = surfaceBuilder.withShape(shape).makeSurface(
					row, theme::ElevationLevel::Level3);
				surface->setSurfaceConfig(surface->getSurfaceConfig()
											  .setFixedWidth(72.0f)
											  .setFixedHeight(48.0f));
				widgets.push_back(surface);
			}
			entities::Text::Builder textBuilder(registry, *row);
			widgets.push_back(textBuilder.makeText(
				row, "Primary", theme::TypeRole::TitleMedium,
				color(SchemeColorRole::Primary)));
			widgets.push_back(textBuilder.makeText(
				row, "Secondary", theme::TypeRole::TitleMedium,
				color(SchemeColorRole::Secondary)));
			widgets.push_back(
				textBuilder.makeText(row, "Error", theme::TypeRole::TitleMedium,
									 color(SchemeColorRole::Error)));
			entities::Switch::Builder switchBuilder(registry, *row);
			widgets.push_back(switchBuilder.makeSwitch(
				row, entities::Switch::Variant::Enabled, true, "", "Switch"));
			endRow(row, "Sample", widgets);
		}

		// Accessibility: every widget carries a components::Accessibility
		// (label, role, checked state...) that its system keeps up to date.
		// The button reads the one of the checkbox and shows it.
		{
			using Checkbox = entities::Checkbox;
			auto row	   = beginRow();
			Checkbox::Builder checkboxBuilder(registry, *row);
			auto checkbox = checkboxBuilder.makeCheckbox(
				row, Checkbox::State::Unchecked, Checkbox::Variant::Enabled,
				"Accept");

			entities::Text::Builder textBuilder(registry, *row);
			auto readout = textBuilder.makeText(
				row, "Press Read", theme::TypeRole::BodyLarge,
				color(SchemeColorRole::OnSurface));

			Button::Builder buttonBuilder(registry, *row);
			const auto checkboxIdentifier = checkbox->getIdentifier();
			auto *readoutPointer		  = readout.get();
			auto read					  = buttonBuilder.makeButton(
				row, "Read",
				[this, checkboxIdentifier, readoutPointer]() {
					using guillaume::components::Accessibility;

					const auto &accessibility =
						getComponentRegistry().getComponent<Accessibility>(
							checkboxIdentifier);
					std::string checked = "not a checkable widget";

					if (const auto state = accessibility.getChecked()) {
						switch (*state) {
							case guillaume::components::CheckedState::Checked:
								checked = "checked";
								break;
							case guillaume::components::CheckedState::Mixed:
								checked = "mixed";
								break;
							case guillaume::components::CheckedState::Unchecked:
								checked = "unchecked";
								break;
						}
					}
					const std::string text = "label \""
						+ accessibility.getLabel() + "\", " + checked;

					readoutPointer->setContent(text);
					logger("Accessibility: " + text)();
				},
				Button::Color::Tonal, Button::Shape::Round, Button::Size::Small,
				false);
			endRow(row, "Accessibility", { checkbox, read, readout });
		}

		endPage<MeshPage, StoragePage>();
	}

}	 // namespace app::scenes::gallery
