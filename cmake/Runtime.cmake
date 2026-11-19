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

# Python is linked through an import library and loaded from python3XX.dll at
# run time, from the directory of the interpreter. Windows does not search
# there unless the directory is on PATH, so the executable stops at start-up
# with 0xc0000135 (STATUS_DLL_NOT_FOUND). The tests and the run target prepend
# the directory, which also keeps the interpreter's standard library next to
# its own DLL.
set(APP_PYTHON_DIR "")
if(WIN32 AND Python_EXECUTABLE)
    get_filename_component(APP_PYTHON_DIR "${Python_EXECUTABLE}" DIRECTORY)
endif()

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
#
# On Windows the interpreter's directory comes first on PATH, as for the
# tests: the executable loads python3XX.dll from there, outside the runtime
# directory. `cmake -E env` does not resolve target names, hence the explicit
# $<TARGET_FILE:...>.
set(APP_RUN_COMMAND ${APP_NAME})
if(APP_PYTHON_DIR)
    set(APP_RUN_COMMAND
        ${CMAKE_COMMAND} -E env
            --modify "PATH=path_list_prepend:${APP_PYTHON_DIR}"
            $<TARGET_FILE:${APP_NAME}>
    )
endif()

add_custom_target(run
    COMMAND ${APP_RUN_COMMAND}
    WORKING_DIRECTORY ${APP_RUNTIME_DIR}
    USES_TERMINAL
    VERBATIM
)
add_dependencies(run ${APP_NAME}_files)
