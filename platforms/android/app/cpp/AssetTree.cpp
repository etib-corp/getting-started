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

#include "AssetTree.hpp"

#include <array>
#include <cstddef>
#include <fstream>
#include <memory>
#include <sstream>
#include <system_error>
#include <utility>
#include <vector>

namespace app::android
{
	namespace
	{
		/// Name of the file holding the stamp of the copied tree.
		constexpr const char *stampFileName = ".stamp";

		/// Closes an asset when its owner goes out of scope.
		struct AssetCloser {
			/**
			 * @brief Close the asset.
			 * @param asset The asset to close.
			 */
			void operator()(AAsset *asset) const
			{
				AAsset_close(asset);
			}
		};

		using Asset = std::unique_ptr<AAsset, AssetCloser>;

		/**
		 * @brief Open an asset for sequential reading.
		 * @param assets The asset manager of the activity.
		 * @param path The asset path.
		 * @return The asset, or null when it is not in the APK.
		 */
		Asset openAsset(AAssetManager *assets, const std::string &path)
		{
			return Asset(AAssetManager_open(assets, path.c_str(),
											AASSET_MODE_STREAMING));
		}

		/**
		 * @brief Read the lines of a manifest.
		 * @param assets The asset manager of the activity.
		 * @param path The asset path of the manifest.
		 * @param lines Receives the non-empty lines.
		 * @return False when the manifest is missing or unreadable.
		 */
		bool readManifest(AAssetManager *assets, const std::string &path,
						  std::vector<std::string> &lines)
		{
			const Asset asset = openAsset(assets, path);
			std::array<char, 16384> buffer {};
			std::string text;
			int count = 0;

			if (!asset) {
				return false;
			}
			while (
				(count = AAsset_read(asset.get(), buffer.data(), buffer.size()))
				> 0) {
				text.append(buffer.data(), static_cast<std::size_t>(count));
			}
			if (count < 0) {
				return false;
			}

			std::istringstream stream(text);
			for (std::string line; std::getline(stream, line);) {
				if (!line.empty() && line.back() == '\r') {
					line.pop_back();
				}
				if (!line.empty()) {
					lines.push_back(std::move(line));
				}
			}
			return true;
		}

		/**
		 * @brief Tell whether a manifest entry stays inside the destination.
		 * @param entry The relative path read from the manifest.
		 * @return False for an absolute path or one that climbs with `..`.
		 */
		bool isContained(const std::filesystem::path &entry)
		{
			if (entry.is_absolute()) {
				return false;
			}
			for (const std::filesystem::path &part: entry) {
				if (part == "..") {
					return false;
				}
			}
			return true;
		}

		/**
		 * @brief Copy one asset to a file.
		 * @param assets The asset manager of the activity.
		 * @param source The asset path.
		 * @param target The file to write; its directory is created.
		 * @return False when the asset is missing or the file was not
		 * written entirely.
		 */
		bool copyAsset(AAssetManager *assets, const std::string &source,
					   const std::filesystem::path &target)
		{
			const Asset asset = openAsset(assets, source);
			std::array<char, 65536> buffer {};
			std::error_code error;
			int count = 0;

			if (!asset) {
				return false;
			}
			std::filesystem::create_directories(target.parent_path(), error);

			std::ofstream file(target, std::ios::binary | std::ios::trunc);
			while (
				(count = AAsset_read(asset.get(), buffer.data(), buffer.size()))
				> 0) {
				file.write(buffer.data(), count);
			}
			file.close();
			return count == 0 && !file.fail();
		}
	}	 // namespace

	std::string extractAssetTree(AAssetManager *assets, const std::string &tree,
								 const std::filesystem::path &destination)
	{
		const std::filesystem::path stampPath = destination / stampFileName;
		std::vector<std::string> lines;
		std::error_code error;

		if (!readManifest(assets, tree + "/manifest.txt", lines)
			|| lines.empty()) {
			return "no manifest in the APK assets for '" + tree + "'";
		}
		const std::string &stamp = lines.front();

		{
			std::ifstream current(stampPath);
			std::string currentStamp;

			if (std::getline(current, currentStamp) && currentStamp == stamp) {
				return {};
			}
		}

		std::filesystem::remove_all(destination, error);
		std::filesystem::create_directories(destination, error);
		if (error) {
			return "cannot create '" + destination.string()
				+ "': " + error.message();
		}

		for (std::size_t index = 1; index < lines.size(); ++index) {
			const std::string &entry = lines[index];

			if (!isContained(entry)) {
				return "'" + entry + "' in the manifest of '" + tree
					+ "' is not a path inside the tree";
			}
			if (!copyAsset(assets, tree + "/" + entry, destination / entry)) {
				return "cannot copy the asset '" + tree + "/" + entry + "'";
			}
		}

		// Written last: an interrupted copy is done again at the next start.
		std::ofstream stampFile(stampPath, std::ios::trunc);
		stampFile << stamp << '\n';
		stampFile.close();
		if (stampFile.fail()) {
			return "cannot write '" + stampPath.string() + "'";
		}
		return {};
	}

}	 // namespace app::android
