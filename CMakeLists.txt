cmake_minimum_required(VERSION 3.28)

# Rename the application here: the executable, the window title default and
# the CTest names all follow this one variable.
set(APP_NAME getting_started)

project(${APP_NAME}
    VERSION 0.1.0
    DESCRIPTION "Starter application built on evan, guillaume and mais"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable CTest at the top level so tests are registered in the root
# CTestTestfile.cmake.
include(CTest)

# 01. Options

# Not named DEV_MODE: evan and mais have an option of that name, which would
# switch them to their own development mode too.
option(APP_DEV_MODE "Compile the application with extra warnings as errors" OFF)
option(APP_ENABLE_GPU_TESTS
    "Register the smoke test that opens a window (needs a display and Vulkan)"
    OFF
)

# 02. Dependencies (evan, guillaume, mais and their shared utility library)

include(cmake/Dependencies.cmake)

# 03. Shared application code (scenes, context, frame helpers)

add_subdirectory(src)

# 04. Scripting adapter
#
# The only code that includes pybind11. It is a library of its own so the tests
# can exercise the Python bindings without opening a window.

add_library(${APP_NAME}_scripting STATIC
    src/scripting/Scripting.cpp
)

target_include_directories(${APP_NAME}_scripting
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${APP_NAME}_scripting
    PUBLIC
        mais
    PRIVATE
        pybind11::embed
)

# 05. Desktop application
#
# The Android application is a separate Gradle project that reuses src/ and
# cmake/Dependencies.cmake: see platforms/android.

add_executable(${APP_NAME}
    platforms/desktop/main.cpp
)

target_compile_definitions(${APP_NAME}
    PRIVATE
        APP_NAME="${APP_NAME}"
        APP_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(${APP_NAME}
    PRIVATE
        ${APP_NAME}_core
        ${APP_NAME}_scripting
        evan
)

set_target_properties(${APP_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(APP_DEV_MODE)
    foreach(app_target ${APP_NAME} ${APP_NAME}_scripting)
        if(MSVC)
            target_compile_options(${app_target} PRIVATE /W4 /WX)
        else()
            target_compile_options(${app_target} PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Werror
            )
        endif()
    endforeach()
endif()

# 06. Runtime directory (assets and scripts next to the executable)

include(cmake/Runtime.cmake)

# 07. Tests

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
