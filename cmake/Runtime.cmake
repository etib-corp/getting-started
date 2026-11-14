# Runtime directory.
#
# The engine and the UI framework load their files by relative path
# (`shaders/...`, `fonts/...`), so everything the application reads at run time
# is gathered in the directory of the executable:
#
#   build/bin/
#     getting_started
#     shaders/    evan's SPIR-V shaders          (from the pinned libraries)
#     fonts/      Roboto and Material Symbols    (from the pinned libraries)
#     textures/   the engine's default texture   (from the pinned libraries)
#     scripts/    this repository's scripts/
#     ...         this repository's assets/
#
# Files from the libraries are taken from their pinned sources, so they always
# match the library version that is linked.

set(APP_RUNTIME_DIR $<TARGET_FILE_DIR:${APP_NAME}>)
set(APP_ENGINE_ASSETS_DIR ${guillaume_SOURCE_DIR}/examples/evan/assets)
set(APP_SYMBOLS_FONT_DIR fonts/Material_Symbols_Outlined)

# Library files change only with the pinned revisions: copy them when the
# executable is linked.
add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${APP_ENGINE_ASSETS_DIR}/shaders
        ${APP_RUNTIME_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${APP_ENGINE_ASSETS_DIR}/textures
        ${APP_RUNTIME_DIR}/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${guillaume_SOURCE_DIR}/fonts
        ${APP_RUNTIME_DIR}/fonts
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${APP_RUNTIME_DIR}/${APP_SYMBOLS_FONT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${APP_ENGINE_ASSETS_DIR}/${APP_SYMBOLS_FONT_DIR}/MaterialSymbolsOutlined-VariableFont_FILL,GRAD,opsz,wght.ttf"
        "${APP_ENGINE_ASSETS_DIR}/${APP_SYMBOLS_FONT_DIR}/MaterialSymbolsOutlined[FILL,GRAD,opsz,wght].codepoints"
        "${APP_ENGINE_ASSETS_DIR}/${APP_SYMBOLS_FONT_DIR}/LICENSE.txt"
        ${APP_RUNTIME_DIR}/${APP_SYMBOLS_FONT_DIR}
    COMMENT "Copying engine shaders and fonts"
    VERBATIM
)

# The application's own files change all the time: copy them on every build,
# so editing a script or an asset never needs a relink.
add_custom_target(${APP_NAME}_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${APP_RUNTIME_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/scripts
        ${APP_RUNTIME_DIR}/scripts
    COMMENT "Copying application assets and scripts"
    VERBATIM
)
add_dependencies(${APP_NAME}_files ${APP_NAME})

# `cmake --build --preset default --target run` builds, copies and starts the
# application from its runtime directory.
add_custom_target(run
    COMMAND ${APP_NAME}
    WORKING_DIRECTORY ${APP_RUNTIME_DIR}
    USES_TERMINAL
    VERBATIM
)
add_dependencies(run ${APP_NAME}_files)
