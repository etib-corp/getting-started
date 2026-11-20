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
 * @brief What every gallery page shares: the column, the rows, the
 * navigation.
 */

#include "scenes/gallery/GalleryPages.hpp"

#include <iostream>

namespace app::scenes::gallery
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;
	using utility::graphic::PoseF;

	std::function<void(void)> logger(const std::string &message)
	{
		return [message]() {
			std::cout << "[gallery] " << message << std::endl;
		};
	}

	utility::graphic::Color32Bit color(SchemeColorRole role)
	{
		return guillaume::getActiveColor(role);
	}

	void Page::onExit(void)
	{
		getEntityRegistry().clearEntities();
		_rows.clear();
		_page.reset();
	}

	void Page::beginPage(const std::string &title)
	{
		// The root of the scene: a column. guillaume places root entities in
		// front of the camera when the scene is entered.
		entities::View::Builder pageBuilder(getComponentRegistry(),
											getEntityRegistry());
		_page = pageBuilder.makeColorView(nullptr, PoseF(),
										  color(SchemeColorRole::Surface), {});
		_page->setAxis(entities::View::Axis::Vertical)
			.setCrossAxisAlignment(entities::View::CrossAxisAlignment::Start)
			.setSpacing(20.0f)
			.setPadding(32.0f)
			.setBorderRadius(28.0f);
		_rows.clear();

		entities::Text::Builder textBuilder(getComponentRegistry(), *_page);
		_rows.push_back(
			textBuilder.makeText(_page, title, theme::TypeRole::HeadlineMedium,
								 color(SchemeColorRole::OnSurface)));
	}

	std::shared_ptr<entities::View> Page::beginRow(void)
	{
		// A row nested in the column: a View owned by a View.
		entities::View::Builder rowBuilder(getComponentRegistry(), *_page);
		auto row = rowBuilder.makeDefaultView(_page, PoseF(), {});

		row->setAxis(entities::View::Axis::Horizontal)
			.setCrossAxisAlignment(entities::View::CrossAxisAlignment::Center)
			.setSpacing(16.0f);
		return row;
	}

	void Page::endRow(const std::shared_ptr<entities::View> &row,
					  const std::string &caption, const Entities &widgets)
	{
		entities::Text::Builder textBuilder(getComponentRegistry(), *row);
		Entities children { textBuilder.makeText(
			row, caption, theme::TypeRole::LabelLarge,
			color(SchemeColorRole::OnSurfaceVariant)) };

		children.insert(children.end(), widgets.begin(), widgets.end());
		row->setEntities(children);
		_rows.push_back(row);
	}

}	 // namespace app::scenes::gallery
