# Files the libraries load at run time, at their pinned revisions.
#
# evan loads `shaders/*.spv` and `textures/default_texture.png`; guillaume
# loads `fonts/Roboto-*.ttf` and the Material Symbols font. The widget gallery
# (src/scenes/gallery/MeshPage.cpp) shows guillaume's sample model. They are taken
# from the fetched sources, so they always match the linked libraries. The
# desktop build copies them next to the executable (cmake/Runtime.cmake), the
# Android build into the APK assets (platforms/android/app/cpp/CMakeLists.txt).
#
# Include after cmake/Dependencies.cmake.

set(APP_ENGINE_ASSETS_DIR ${guillaume_SOURCE_DIR}/examples/evan/assets)
set(APP_ENGINE_SHADERS_DIR ${APP_ENGINE_ASSETS_DIR}/shaders)
set(APP_ENGINE_TEXTURES_DIR ${APP_ENGINE_ASSETS_DIR}/textures)
set(APP_ENGINE_FONTS_DIR ${guillaume_SOURCE_DIR}/fonts)
set(APP_ENGINE_MODELS_DIR ${APP_ENGINE_ASSETS_DIR}/models)

# Only these files of the Material Symbols directory are loaded; the directory
# also holds 70 MB of static fonts that are not.
set(APP_SYMBOLS_FONT_DIR fonts/Material_Symbols_Outlined)
set(APP_SYMBOLS_FONT_FILES
    "${APP_ENGINE_ASSETS_DIR}/${APP_SYMBOLS_FONT_DIR}/MaterialSymbolsOutlined-VariableFont_FILL,GRAD,opsz,wght.ttf"
    "${APP_ENGINE_ASSETS_DIR}/${APP_SYMBOLS_FONT_DIR}/MaterialSymbolsOutlined[FILL,GRAD,opsz,wght].codepoints"
    "${APP_ENGINE_ASSETS_DIR}/${APP_SYMBOLS_FONT_DIR}/LICENSE.txt"
)
