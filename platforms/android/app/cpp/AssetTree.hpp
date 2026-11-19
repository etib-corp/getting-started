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

#include <android/asset_manager.h>

#include <filesystem>
#include <string>

/**
 * @file
 * @brief Copies a directory of the APK assets to the file system.
 *
 * Python imports modules from real files, and the assets of an APK are not
 * files. The build packages the Python standard library and the scripts as
 * assets, each tree with a `manifest.txt` written by cmake/AssetTree.cmake:
 * a stamp identifying the content on the first line, then one relative path
 * per line. The manifest is needed because native code cannot list the
 * sub-directories of an asset directory.
 */

namespace app::android
{
	/**
	 * @brief Copy an asset tree to a directory, unless it is already there.
	 *
	 * The stamp of the manifest is kept next to the copied files. When it
	 * matches, nothing is copied, so only the first start after an
	 * installation or an update pays for the copy. Otherwise the destination
	 * is emptied first: no file of a previous version is left behind.
	 * @param assets The asset manager of the activity.
	 * @param tree The asset directory holding `manifest.txt`.
	 * @param destination The directory to fill; created when missing.
	 * @return An empty string on success, the reason of the failure
	 * otherwise.
	 */
	std::string extractAssetTree(AAssetManager *assets, const std::string &tree,
								 const std::filesystem::path &destination);

}	 // namespace app::android
