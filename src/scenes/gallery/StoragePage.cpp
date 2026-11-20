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
 * @brief Gallery page 11: storage.
 *
 * Two counters with the same code and two storages: guillaume::LocalStorage
 * keeps its value in a file and finds it again at the next start,
 * guillaume::SessionStorage keeps it in memory until the application exits.
 */

#include "scenes/gallery/GalleryPages.hpp"

#include <string>

#include <guillaume/local_storage.hpp>
#include <guillaume/session_storage.hpp>
#include <guillaume/storage.hpp>

namespace app::scenes::gallery
{
	namespace entities = guillaume::entities;
	namespace theme	   = guillaume::theme;

	using guillaume::SchemeColorRole;

	namespace
	{
		/// Key of the counter, in both storages.
		const std::string counterKey = "gallery.clicks";

		/**
		 * @brief The persistent storage of the page.
		 *
		 * Opened on first use, in the default file of guillaume
		 * (`.guillaume-local-storage.db` in the working directory, which is
		 * the directory of the executable on desktop). When the file cannot
		 * be opened, the storage works in memory and isPersistent() is false.
		 * @return The storage.
		 */
		guillaume::LocalStorage &localStorage(void)
		{
			static guillaume::LocalStorage storage;

			return storage;
		}

		/**
		 * @brief The in-memory storage of the page.
		 *
		 * It lives as long as the process, so the counter survives a change
		 * of page but not a restart.
		 * @return The storage.
		 */
		guillaume::SessionStorage &sessionStorage(void)
		{
			static guillaume::SessionStorage storage;

			return storage;
		}

		/**
		 * @brief Read the counter of a storage.
		 * @param storage The storage.
		 * @return The counter; 0 when the key is absent.
		 */
		int readCounter(guillaume::Storage &storage)
		{
			return storage.getItemAs<int>(counterKey).value_or(0);
		}

		/**
		 * @brief Text of a counter line.
		 * @param storage The storage holding the counter.
		 * @return For example "3 click(s) stored".
		 */
		std::string describe(guillaume::Storage &storage)
		{
			return std::to_string(readCounter(storage)) + " click(s) stored";
		}
	}	 // namespace

	void StoragePage::onEnter(void)
	{
		using Button   = entities::Button;
		auto &registry = getComponentRegistry();

		beginPage("11 / 11  Storage");

		// One row per storage: the stored value, Add and Clear. Both rows
		// run the same code on a guillaume::Storage.
		const auto addRow = [this, &registry](const std::string &caption,
											  const std::string &name,
											  guillaume::Storage &storage) {
			auto row = beginRow();
			entities::Text::Builder textBuilder(registry, *row);
			auto value = textBuilder.makeText(
				row, describe(storage), theme::TypeRole::BodyLarge,
				color(SchemeColorRole::OnSurface));
			auto *valuePointer	 = value.get();
			auto *storagePointer = &storage;

			Button::Builder buttonBuilder(registry, *row);
			auto add = buttonBuilder.makeButton(
				row, "Add",
				[valuePointer, storagePointer, name]() {
					// setItem() accepts numbers; getItemAs<int>() reads
					// them back.
					storagePointer->setItem(counterKey,
											readCounter(*storagePointer) + 1);
					valuePointer->setContent(describe(*storagePointer));
					logger(name + ": " + describe(*storagePointer))();
				},
				Button::Color::Filled, Button::Shape::Round,
				Button::Size::Small, false);
			auto clear = buttonBuilder.makeButton(
				row, "Clear",
				[valuePointer, storagePointer, name]() {
					storagePointer->removeItem(counterKey);
					valuePointer->setContent(describe(*storagePointer));
					logger(name + ": cleared")();
				},
				Button::Color::Outlined, Button::Shape::Round,
				Button::Size::Small, false);
			endRow(row, caption, { value, add, clear });
		};

		const bool persistent = localStorage().isPersistent();

		addRow(persistent ? "LocalStorage (kept after a restart)"
						  : "LocalStorage (in memory: the file could not be "
							"opened)",
			   "LocalStorage", localStorage());
		addRow("SessionStorage (lost at exit)", "SessionStorage",
			   sessionStorage());

		endPage<ThemePage, HomeScene>();
	}

}	 // namespace app::scenes::gallery
