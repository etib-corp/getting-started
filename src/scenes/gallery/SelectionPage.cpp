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
 * @brief Gallery page 2: selection.
 *
 * Checkbox, RadioButton, RadioGroup, Switch and Chip.
 */

#include "scenes/gallery/GalleryPages.hpp"

#include <string>

#include <guillaume/theme.hpp>

#include <guillaume/entities/checkbox.hpp>
#include <guillaume/entities/chip.hpp>
#include <guillaume/entities/radio_button.hpp>
#include <guillaume/entities/radio_group.hpp>
#include <guillaume/entities/switch.hpp>

namespace app::scenes::gallery
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;

	void SelectionPage::onEnter(void)
	{
		auto &registry = getComponentRegistry();

		beginPage("2 / 11  Selection");

		// Checkboxes: the three states and the three variants.
		{
			using Checkbox = entities::Checkbox;
			auto row	   = beginRow();
			Checkbox::Builder builder(registry, *row);
			Entities widgets;
			auto onChange = [](Checkbox::State state) {
				logger("Checkbox state: "
					   + std::to_string(static_cast<int>(state)))();
			};
			auto unchecked =
				builder.makeCheckbox(row, Checkbox::State::Unchecked,
									 Checkbox::Variant::Enabled, "Unchecked");
			unchecked->setOnValueChange(onChange);
			auto checked =
				builder.makeCheckbox(row, Checkbox::State::Checked,
									 Checkbox::Variant::Enabled, "Checked");
			checked->setOnValueChange(onChange);
			auto mixed = builder.makeCheckbox(
				row, Checkbox::State::Indeterminate, Checkbox::Variant::Enabled,
				"Indeterminate");
			mixed->setOnValueChange(onChange);
			widgets.push_back(unchecked);
			widgets.push_back(checked);
			widgets.push_back(mixed);
			widgets.push_back(
				builder.makeCheckbox(row, Checkbox::State::Checked,
									 Checkbox::Variant::Error, "Error"));
			widgets.push_back(
				builder.makeCheckbox(row, Checkbox::State::Checked,
									 Checkbox::Variant::Disabled, "Disabled"));
			endRow(row, "Checkbox", widgets);
		}

		// Radio buttons, and a radio group that keeps one selection.
		{
			using Radio = entities::RadioButton;
			auto row	= beginRow();
			Radio::Builder builder(registry, *row);
			Entities widgets;
			auto enabled = builder.makeRadioButton(row, Radio::Variant::Enabled,
												   "Enabled");
			enabled->setOnPress(logger("Radio pressed"));
			widgets.push_back(enabled);
			widgets.push_back(
				builder.makeRadioButton(row, Radio::Variant::Error, "Error"));
			widgets.push_back(builder.makeRadioButton(
				row, Radio::Variant::Disabled, "Disabled"));

			entities::RadioGroup::Builder groupBuilder(registry, *row);
			auto group = groupBuilder.makeRadioGroup(
				row, { "Small", "Medium", "Large" });
			widgets.push_back(group);
			endRow(row, "Radio", widgets);
		}

		// Switches.
		{
			using Switch = entities::Switch;
			auto row	 = beginRow();
			Switch::Builder builder(registry, *row);
			Entities widgets;
			auto off = builder.makeSwitch(row, Switch::Variant::Enabled, false,
										  "", "Off");
			off->setOnValueChange([](bool on) {
				logger(std::string("Switch: ") + (on ? "on" : "off"))();
			});
			widgets.push_back(off);
			widgets.push_back(builder.makeSwitch(row, Switch::Variant::Enabled,
												 true, "", "On"));
			widgets.push_back(builder.makeSwitch(row, Switch::Variant::WithIcon,
												 true, "check", "With icon"));
			widgets.push_back(builder.makeSwitch(row, Switch::Variant::Disabled,
												 true, "", "Disabled"));
			endRow(row, "Switch", widgets);
		}

		// Chips: the four variants.
		{
			using Chip = entities::Chip;
			auto row   = beginRow();
			Chip::Builder builder(registry, *row);
			Entities widgets;
			auto assist = builder.makeChip(row, Chip::Variant::Assist, "Assist",
										   "event", false);
			assist->setOnPress(logger("Assist chip pressed"));
			widgets.push_back(assist);
			widgets.push_back(builder.makeChip(row, Chip::Variant::Filter,
											   "Filter", "", true));
			auto input =
				builder.makeChip(row, Chip::Variant::Input, "Input", "", false);
			input->setOnClose(logger("Input chip closed"));
			widgets.push_back(input);
			widgets.push_back(builder.makeChip(row, Chip::Variant::Suggestion,
											   "Suggestion", "", false));
			auto outlined = builder.makeChip(row, Chip::Variant::Filter,
											 "Outlined", "", false);
			outlined->setMode(Chip::Mode::Outlined);
			widgets.push_back(outlined);
			endRow(row, "Chip", widgets);
		}

		endPage<ButtonsPage, TextPage>();
	}

}	 // namespace app::scenes::gallery
