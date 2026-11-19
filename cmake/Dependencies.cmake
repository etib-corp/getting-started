# Dependencies of the application.
#
# The three libraries are fetched from Git at the revisions pinned below, so
# every developer and CI run builds the same code. To move to a newer release,
# change a revision here, rebuild and run the application.
#
# To work on a library and the application together, point CMake at a local
# checkout instead of the pinned revision:
#
# cmake --preset default -DFETCHCONTENT_SOURCE_DIR_GUILLAUME=/path/to/guillaume
#
# The `siblings` preset does this for evan, guillaume and mais checked out next
# to this repository.

include(FetchContent)

# evan and guillaume both build against utility's main branch. It is declared
# first so that this revision is the one they both get.
set(APP_UTILITY_REVISION dde6d86ad13972f208a1cdb90058bfd9f1a132d3)
set(APP_EVAN_REVISION v2026.4.7)

# guillaume 2026.6.1 (the revision has no tag yet).
set(APP_GUILLAUME_REVISION 15a2426dfa7892823126cfc6eb653d81f06fd093)

# mais has no release tag yet. The application needs ScriptArgument::native(),
# which is on main.
set(APP_MAIS_REVISION bddc4f1b6dcc142f3d0295ab8b36174db3f1db5c)

# evan builds exactly one backend and one platform. The application is a
# desktop one: GLFW, on the platform of the host.
set(BUILD_FOR_GLFW ON CACHE BOOL "" FORCE)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(BUILD_FOR_MACOS ON CACHE BOOL "" FORCE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(BUILD_FOR_LINUX ON CACHE BOOL "" FORCE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(BUILD_FOR_WINDOWS ON CACHE BOOL "" FORCE)
else()
    message(FATAL_ERROR
        "${APP_NAME} supports macOS, Linux and Windows "
        "(got ${CMAKE_SYSTEM_NAME})."
    )
endif()

# Windows: two settings the pinned revisions force on what they build have to
# be matched here, or a Debug build of the application does not link.
#
# - utility and evan set CMAKE_MSVC_RUNTIME_LIBRARY to MultiThreadedDLL in
# their cmake/ConfigWindows.cmake, in every configuration. The application
# keeps CMake's per-configuration default and would be built with /MDd in
# Debug: https://github.com/etib-corp/utility/issues/131 and
# https://github.com/etib-corp/evan/issues/183.
# - utility's vendored OpenAL Soft enables C++20 modules for the Visual Studio
# generator, which MSBuild cannot build ("error C2230: could not find module
# 'alc.context'"): https://github.com/etib-corp/utility/issues/132.
#
# Both problems belong to the libraries; until they are fixed, the two
# settings below keep a fresh checkout buildable on Windows.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    set(ALSOFT_ENABLE_MODULES OFF CACHE BOOL "Use C++20 modules in OpenAL Soft"
        FORCE
    )
endif()

# The libraries are consumed as libraries only: their tests, examples,
# benchmarks and documentation are built and run in their own repositories.
set(APP_BUILD_TESTING ${BUILD_TESTING})
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BUILD_DOCS OFF CACHE BOOL "" FORCE)

# EXCLUDE_FROM_ALL keeps the libraries' own install rules and unused targets
# out of the default build; what the application links is still built.
# SYSTEM keeps warnings from the libraries' headers out of the application's
# build, so APP_DEV_MODE only judges the application's own code.
FetchContent_Declare(utility
    GIT_REPOSITORY https://github.com/etib-corp/utility.git
    GIT_TAG ${APP_UTILITY_REVISION}
    GIT_PROGRESS TRUE
    EXCLUDE_FROM_ALL
    SYSTEM
)

FetchContent_Declare(evan
    GIT_REPOSITORY https://github.com/etib-corp/evan.git
    GIT_TAG ${APP_EVAN_REVISION}
    GIT_PROGRESS TRUE
    EXCLUDE_FROM_ALL
    SYSTEM
)

FetchContent_Declare(guillaume
    GIT_REPOSITORY https://github.com/etib-corp/guillaume.git
    GIT_TAG ${APP_GUILLAUME_REVISION}
    GIT_PROGRESS TRUE
    EXCLUDE_FROM_ALL
    SYSTEM
)

FetchContent_Declare(mais
    GIT_REPOSITORY https://github.com/etib-corp/mais.git
    GIT_TAG ${APP_MAIS_REVISION}
    GIT_PROGRESS TRUE
    EXCLUDE_FROM_ALL
    SYSTEM
)

FetchContent_MakeAvailable(utility evan guillaume mais)

set(BUILD_TESTING ${APP_BUILD_TESTING} CACHE BOOL "" FORCE)

# mais fetches pybind11 when it is not installed, which makes the target
# visible here. When mais found an installed pybind11 instead, the imported
# target is scoped to mais's directory and has to be found again.
if(NOT TARGET pybind11::embed)
    find_package(pybind11 CONFIG REQUIRED)
endif()
