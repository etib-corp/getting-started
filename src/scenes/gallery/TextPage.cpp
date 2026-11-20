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
 * @brief Gallery page 3: text and input.
 *
 * Text (type scale), Icon, Image, TextField and Search.
 */

#include "scenes/gallery/GalleryPages.hpp"

#include <string>
#include <utility>
#include <vector>

#include <guillaume/theme.hpp>

#include <guillaume/entities/icon.hpp>
#include <guillaume/entities/image.hpp>
#include <guillaume/entities/search.hpp>
#include <guillaume/entities/text.hpp>
#include <guillaume/entities/text_field.hpp>

namespace app::scenes::gallery
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;

	void TextPage::onEnter(void)
	{
		auto &registry = getComponentRegistry();

		beginPage("3 / 11  Text and input");

		// The type scale, one sample per group.
		{
			auto row = beginRow();
			entities::Text::Builder builder(registry, *row);
			Entities widgets;
			const std::vector<std::pair<std::string, theme::TypeRole>> roles {
				{ "Display", theme::TypeRole::DisplaySmall },
				{ "Headline", theme::TypeRole::HeadlineSmall },
				{ "Title", theme::TypeRole::TitleMedium },
				{ "Body", theme::TypeRole::BodyMedium },
				{ "Label", theme::TypeRole::LabelSmall },
			};
			for (const auto &[label, role]: roles) {
				widgets.push_back(builder.makeText(
					row, label, role, color(SchemeColorRole::OnSurface)));
			}
			endRow(row, "Text", widgets);
		}

		// Icons and an image.
		{
			auto row = beginRow();
			entities::Icon::Builder builder(registry, *row);
			Entities widgets;
			for (const std::string glyph:
				 { "home", "search", "settings", "favorite", "delete" }) {
				widgets.push_back(builder.makeIcon(
					row, glyph, 32.0f, color(SchemeColorRole::Primary),
					guillaume::components::Glyph::Style::Outlined));
			}
			entities::Image::Builder imageBuilder(registry, *row);
			auto image =
				imageBuilder.makeImage(row, "textures/default_texture.png");
			image->setSize(64.0f, 64.0f);
			widgets.push_back(image);
			endRow(row, "Icon, Image", widgets);
		}

		// Text fields: filled, outlined, with an error.
		{
			using TextField = entities::TextField;
			auto row		= beginRow();
			TextField::Builder builder(registry, *row);
			Entities widgets;
			auto filled =
				builder.makeTextField(row, TextField::Variant::Filled, "Name",
									  "person", "", "Click, then type", false);
			filled->setOnChange([](const std::string &text) {
				logger("Text field: " + text)();
			});
			widgets.push_back(filled);
			widgets.push_back(builder.makeTextField(
				row, TextField::Variant::Outlined, "Email", "", "mail", "",
				false, "someone@example.com"));
			auto error =
				builder.makeTextField(row, TextField::Variant::Filled, "Age",
									  "", "error", "", true, "abc");
			error->setErrorText("Enter a number");
			widgets.push_back(error);
			endRow(row, "TextField", widgets);
		}

		// Search bar.
		{
			auto row = beginRow();
			entities::Search::Builder builder(registry, *row);
			auto search =
				builder.makeSearch(row, entities::Search::Variant::SearchBar,
								   "Search the gallery");
			search->setOnIconPress(logger("Search icon pressed"));
			endRow(row, "Search", { search });
		}

		endPage<SelectionPage, ContainersPage>();
	}

}	 // namespace app::scenes::gallery
