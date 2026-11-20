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
 * @brief Gallery page 4: surfaces and cards.
 *
 * Surface (elevation levels) and Card.
 */

#include "scenes/gallery/GalleryPages.hpp"

#include <vector>

#include <guillaume/theme.hpp>

#include <guillaume/entities/card.hpp>
#include <guillaume/entities/surface.hpp>

namespace app::scenes::gallery
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;

	void ContainersPage::onEnter(void)
	{
		auto &registry = getComponentRegistry();

		beginPage("4 / 11  Surfaces and cards");

		// Surfaces at three elevation levels, flat and elevated.
		{
			using Surface = entities::Surface;
			auto row	  = beginRow();
			Surface::Builder builder(registry, *row);
			Entities widgets;
			const std::vector<theme::ElevationLevel> levels {
				theme::ElevationLevel::Level0, theme::ElevationLevel::Level1,
				theme::ElevationLevel::Level3, theme::ElevationLevel::Level5
			};
			for (const auto level: levels) {
				auto surface = builder.withShape(theme::ShapeRole::Medium)
								   .makeSurface(row, level);
				surface->setSurfaceConfig(surface->getSurfaceConfig()
											  .setFixedWidth(72.0f)
											  .setFixedHeight(48.0f));
				widgets.push_back(surface);
			}
			auto flat = builder.makeSurface(row, theme::ElevationLevel::Level3,
											Surface::Mode::Flat);
			flat->setSurfaceConfig(
				flat->getSurfaceConfig().setFixedWidth(72.0f).setFixedHeight(
					48.0f));
			widgets.push_back(flat);
			endRow(row, "Surface", widgets);
		}

		// Cards: the three variants, the last one with every section.
		{
			using Card = entities::Card;
			auto row   = beginRow();
			Card::Builder builder(registry, *row);
			Entities widgets;
			auto elevated = builder.makeCard(row, Card::Variant::Elevated);
			elevated->setTitle("Elevated", "Card");
			elevated->setOnPress(logger("Elevated card pressed"));
			widgets.push_back(elevated);
			auto filled = builder.makeCard(row, Card::Variant::Filled);
			filled->setTitle("Filled", "Card");
			widgets.push_back(filled);
			auto outlined = builder.makeCard(row, Card::Variant::Outlined);
			outlined->setCover("textures/default_texture.png");
			outlined->setTitle("Outlined", "With every section");
			outlined->setContent("Cover, title, content and actions.");
			outlined->addAction("Share", logger("Card action: share"));
			outlined->addAction("Open", logger("Card action: open"));
			widgets.push_back(outlined);
			endRow(row, "Card", widgets);
		}

		endPage<TextPage, ListsPage>();
	}

}	 // namespace app::scenes::gallery
