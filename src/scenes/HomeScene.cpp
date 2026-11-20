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

#include <cstdlib>
#include <string>

#include <guillaume/theme.hpp>

#include <guillaume/entities/button.hpp>

#include <utility/graphic/pose.hpp>

#include "AppContext.hpp"
#include "scenes/gallery/GalleryPages.hpp"

namespace app::scenes
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;
	using utility::graphic::PoseF;

	namespace
	{
		/**
		 * @brief Open the gallery page named by GETTING_STARTED_PAGE.
		 *
		 * `GETTING_STARTED_PAGE=6 ./build/bin/getting_started` starts on page
		 * 6 of the gallery (1 to 11, see scenes/gallery/GalleryPages.hpp).
		 * Only on the first entry of the home scene, so the Home buttons work
		 * normally afterwards.
		 * @param home The home scene.
		 */
		void openRequestedPage(HomeScene &home)
		{
			static bool done  = false;
			const char *value = std::getenv("GETTING_STARTED_PAGE");

			if (done || value == nullptr) {
				return;
			}
			done = true;
			switch (std::atoi(value)) {
				case 1:
					home.goToScene<gallery::ButtonsPage>();
					break;
				case 2:
					home.goToScene<gallery::SelectionPage>();
					break;
				case 3:
					home.goToScene<gallery::TextPage>();
					break;
				case 4:
					home.goToScene<gallery::ContainersPage>();
					break;
				case 5:
					home.goToScene<gallery::ListsPage>();
					break;
				case 6:
					home.goToScene<gallery::FeedbackPage>();
					break;
				case 7:
					home.goToScene<gallery::NavigationPage>();
					break;
				case 8:
					home.goToScene<gallery::DrawerPage>();
					break;
				case 9:
					home.goToScene<gallery::MeshPage>();
					break;
				case 10:
					home.goToScene<gallery::ThemePage>();
					break;
				case 11:
					home.goToScene<gallery::StoragePage>();
					break;
				default:
					break;
			}
		}
	}	 // namespace

	void HomeScene::onEnter(void)
	{
		openRequestedPage(*this);

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
			_page, "A starter built on evan, guillaume and mais",
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

		// Scene transitions: each button leaves this scene for a page of the
		// widget gallery (src/scenes/gallery/). The switch happens at the end
		// of the frame: onExit() here, then onEnter() there.
		auto gallery = buttonBuilder.makeIconButton(
			_page, "Widget gallery", "widgets",
			guillaume::components::Glyph::Style::Outlined,
			[this]() {
				goToScene<gallery::ButtonsPage>();
			},
			entities::Button::Color::Tonal, entities::Button::Shape::Round,
			entities::Button::Size::Small, false);
		auto themes = buttonBuilder.makeIconButton(
			_page, "Theme and storage", "palette",
			guillaume::components::Glyph::Style::Outlined,
			[this]() {
				goToScene<gallery::ThemePage>();
			},
			entities::Button::Color::Outlined, entities::Button::Shape::Round,
			entities::Button::Size::Small, false);

		_page->setEntities(
			{ title, subtitle, _status, button, gallery, themes });

		// Let the host loop update the status line while this scene is shown.
		context().showStatus = [this](const std::string &status) {
			_status->setContent(status);
		};
		context().shownStatus.clear();
	}

	void HomeScene::onExit(void)
	{
		context().showStatus = nullptr;
		getEntityRegistry().clearEntities();
		_status.reset();
		_page.reset();
	}

}	 // namespace app::scenes
