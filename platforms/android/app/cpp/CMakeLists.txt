# Native part of the Android application (Meta headsets, OpenXR).
#
# This project is configured by Gradle, not by the root CMakeLists.txt. It
# reuses the root's pinned dependencies (cmake/Dependencies.cmake) and shared
# application code (src/), and adds the Android entry point.

cmake_minimum_required(VERSION 3.28)

# Same name as in the root CMakeLists.txt.
set(APP_NAME getting_started)

project(${APP_NAME}_android LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(APP_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../..")
cmake_path(NORMAL_PATH APP_ROOT_DIR)

# 01. Cross-compilation fixes for the dependencies

# Some third-party checks use try_run() to probe GNU strerror_r, which cannot
# run when cross-compiling. Bionic does not have the GNU variant.
if(CMAKE_CROSSCOMPILING)
    set(HAVE_GNU_STRERROR_R_EXITCODE "1" CACHE STRING "" FORCE)
    set(HAVE_GNU_STRERROR_R_EXITCODE__TRYRUN_OUTPUT "" CACHE STRING "" FORCE)
endif()

# OpenAL Soft enables C++20 module targets in C++20 mode, which need a newer
# Ninja than the one bundled with the Android SDK.
set(ALSOFT_ENABLE_MODULES OFF CACHE BOOL "" FORCE)

# 02. Dependencies and shared application code
#
# On Android, Dependencies.cmake selects evan's OpenXR backend and sets up the
# Python published on python.org for mais (cmake/AndroidPython.cmake).

include(${APP_ROOT_DIR}/cmake/Dependencies.cmake)
include(${APP_ROOT_DIR}/cmake/EngineAssets.cmake)

add_subdirectory(${APP_ROOT_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/app-core)

# 03. Android glue (provides ANativeActivity_onCreate and calls android_main)

add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

target_include_directories(native_app_glue
    PUBLIC
        ${ANDROID_NDK}/sources/android/native_app_glue
)

# 04. The shared library loaded by android.app.NativeActivity
#
# Its name is the `android.app.lib_name` value of AndroidManifest.xml.

add_library(${PROJECT_NAME} SHARED
    AssetTree.cpp
    main.cpp
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        APP_NAME="${APP_NAME}"
        APP_PYTHON_STDLIB_SUBDIR="${APP_PYTHON_ANDROID_STDLIB_SUBDIR}"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${APP_NAME}_core
        ${APP_NAME}_scripting
        evan
        native_app_glue
        android
        log
)

# The entry point is only referenced by the system: keep it at link time.
target_link_options(${PROJECT_NAME}
    PRIVATE
        -u ANativeActivity_onCreate
)

# 05. Engine assets
#
# Copied on every native build into the directory Gradle packages as APK
# assets (APP_ENGINE_ASSETS_OUTPUT_DIR is passed by app/build.gradle.kts).

if(NOT APP_ENGINE_ASSETS_OUTPUT_DIR)
    set(APP_ENGINE_ASSETS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/engine-assets)
endif()

add_custom_target(${APP_NAME}_engine_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${APP_ENGINE_SHADERS_DIR}
        ${APP_ENGINE_ASSETS_OUTPUT_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${APP_ENGINE_TEXTURES_DIR}
        ${APP_ENGINE_ASSETS_OUTPUT_DIR}/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${APP_ENGINE_FONTS_DIR}
        ${APP_ENGINE_ASSETS_OUTPUT_DIR}/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${APP_ENGINE_MODELS_DIR}
        ${APP_ENGINE_ASSETS_OUTPUT_DIR}/models
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${APP_ENGINE_ASSETS_OUTPUT_DIR}/${APP_SYMBOLS_FONT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${APP_SYMBOLS_FONT_FILES}
        ${APP_ENGINE_ASSETS_OUTPUT_DIR}/${APP_SYMBOLS_FONT_DIR}
    COMMENT "Copying engine shaders and fonts to the APK assets"
    VERBATIM
)

# Gradle builds the library targets, not `all`: tie the copy to the library.
add_dependencies(${PROJECT_NAME} ${APP_NAME}_engine_assets)

# 06. Python runtime and scripts
#
# Python reads its standard library and the scripts from real files, which an
# APK does not have. Both trees are packaged as assets with a manifest
# (cmake/AssetTree.cmake) and the host copies them to the application's
# private directory at start-up:
#
#   assets/python/lib/python3.x/   the standard library, without its tests
#   assets/scripts/                this repository's scripts/
#
# The standard library only changes with the Python version, so its stamp is
# the checksum of the python.org package. The scripts are hashed on every
# build.

if(NOT APP_PYTHON_ASSETS_OUTPUT_DIR)
    set(APP_PYTHON_ASSETS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/python-assets)
endif()
if(NOT APP_PYTHON_LIBS_OUTPUT_DIR)
    set(APP_PYTHON_LIBS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/python-libs)
endif()

# A list given to `cmake -D` on a command line: escape its separators.
string(REPLACE ";" "\\;" app_python_stdlib_exclude
    "${APP_PYTHON_ANDROID_STDLIB_EXCLUDE}"
)

add_custom_target(${APP_NAME}_python_assets
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${APP_PYTHON_ANDROID_STDLIB_DIR}
        -DOUTPUT_DIR=${APP_PYTHON_ASSETS_OUTPUT_DIR}/python/${APP_PYTHON_ANDROID_STDLIB_SUBDIR}
        -DEXCLUDE=${app_python_stdlib_exclude}
        -DSTAMP=${APP_PYTHON_ANDROID_VERSION}-${APP_PYTHON_ANDROID_SHA256}
        -P ${APP_ROOT_DIR}/cmake/AssetTree.cmake
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${APP_ROOT_DIR}/scripts
        -DOUTPUT_DIR=${APP_PYTHON_ASSETS_OUTPUT_DIR}/scripts
        -DEXCLUDE=__pycache__/
        -P ${APP_ROOT_DIR}/cmake/AssetTree.cmake
    # The libraries the standard library's extension modules load. Gradle
    # packages this directory as native libraries (see app/build.gradle.kts).
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${APP_PYTHON_LIBS_OUTPUT_DIR}/${ANDROID_ABI}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${APP_PYTHON_ANDROID_RUNTIME_LIBRARIES}
        ${APP_PYTHON_LIBS_OUTPUT_DIR}/${ANDROID_ABI}
    COMMENT "Copying the Python runtime and the scripts to the APK"
    VERBATIM
)

add_dependencies(${PROJECT_NAME} ${APP_NAME}_python_assets)
