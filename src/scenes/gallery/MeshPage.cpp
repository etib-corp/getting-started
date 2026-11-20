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
 * @brief Gallery page 9: 3D mesh.
 *
 * Mesh: a 3D model placed in the layout.
 */

#include "scenes/gallery/GalleryPages.hpp"

#include <guillaume/theme.hpp>

#include <guillaume/entities/mesh.hpp>

namespace app::scenes::gallery
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;

	void MeshPage::onEnter(void)
	{
		beginPage("9 / 11  3D mesh");

		auto row = beginRow();
		entities::Mesh::Builder meshBuilder(getComponentRegistry(), *row);
		// No texture: the engine draws an untextured model flat, which
		// shows its silhouette on the light page.
		auto teapot = meshBuilder.makeMesh(row, "models/teapot.obj", "");
		teapot->setFitSize(160.0f);
		endRow(row, "Mesh", { teapot });

		endPage<DrawerPage, ThemePage>();
	}

}	 // namespace app::scenes::gallery
