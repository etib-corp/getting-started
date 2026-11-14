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
# On Android, Dependencies.cmake selects evan's OpenXR backend and leaves mais
# out (no Python in the Android build).

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
    main.cpp
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        APP_NAME="${APP_NAME}"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${APP_NAME}_core
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
