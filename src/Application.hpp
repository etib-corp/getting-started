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

#pragma once

#include <guillaume/application.hpp>

#include "scenes/HomeScene.hpp"
#include "scenes/gallery/GalleryPages.hpp"

/**
 * @file
 * @brief The scenes of the application, shared by every platform host.
 */

namespace app
{
	/**
	 * @brief The guillaume application with every scene of this application.
	 *
	 * List each scene class here; the first one is shown at start-up. The
	 * desktop and Android hosts both create this type, so a scene added here
	 * exists on every platform.
	 */
	using Application = guillaume::Application<
		scenes::HomeScene, scenes::gallery::ButtonsPage,
		scenes::gallery::SelectionPage, scenes::gallery::TextPage,
		scenes::gallery::ContainersPage, scenes::gallery::ListsPage,
		scenes::gallery::FeedbackPage, scenes::gallery::NavigationPage,
		scenes::gallery::DrawerPage, scenes::gallery::MeshPage,
		scenes::gallery::ThemePage, scenes::gallery::StoragePage>;

}	 // namespace app
