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

#include "scenes/HomeScene.hpp"

#include <string>

#include <guillaume/theme.hpp>

#include <guillaume/entities/button.hpp>

#include <utility/graphic/pose.hpp>

#include "AppContext.hpp"

namespace app::scenes
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;
	using utility::graphic::PoseF;

	void HomeScene::onEnter(void)
	{
		auto &registry = getComponentRegistry();

		// The root of the scene: a column. guillaume places root entities in
		// front of the camera when the scene is entered.
		entities::View::Builder pageBuilder(registry, getEntityRegistry());
		_page = pageBuilder.makeColorView(
			nullptr, PoseF(),
			guillaume::getActiveColor(SchemeColorRole::Surface), {});
		_page->setAxis(entities::View::Axis::Vertical)
			.setCrossAxisAlignment(entities::View::CrossAxisAlignment::Start)
			.setSpacing(20.0f)
			.setPadding(32.0f)
			.setBorderRadius(28.0f);

		// Children are owned by the column: their builders take it as the
		// registry.
		entities::Text::Builder textBuilder(registry, *_page);
		auto title = textBuilder.makeText(
			_page, "Getting started", theme::TypeRole::HeadlineMedium,
			guillaume::getActiveColor(SchemeColorRole::OnSurface));
		auto subtitle = textBuilder.makeText(
			_page, "evan renders, guillaume lays out, mais runs scripts/app.py",
			theme::TypeRole::BodyLarge,
			guillaume::getActiveColor(SchemeColorRole::OnSurfaceVariant));
		_status = textBuilder.makeText(
			_page, "Waiting for the script...", theme::TypeRole::BodyLarge,
			guillaume::getActiveColor(SchemeColorRole::OnSurface));

		// A click only records a fact in the shared context. The script reads
		// it on its next on_update() and answers through the status line.
		entities::Button::Builder buttonBuilder(registry, *_page);
		auto button = buttonBuilder.makeButton(
			_page, "Click me",
			[]() {
				++context().scripting.frame.clicks;
			},
			entities::Button::Color::Filled, entities::Button::Shape::Round,
			entities::Button::Size::Small, false);

		_page->setEntities({ title, subtitle, _status, button });

		// Let the host loop update the status line while this scene is shown.
		context().showStatus = [this](const std::string &status) {
			_status->setContent(status);
		};
	}

	void HomeScene::onExit(void)
	{
		context().showStatus = nullptr;
		getEntityRegistry().clearEntities();
		_status.reset();
		_page.reset();
	}

}	 // namespace app::scenes
