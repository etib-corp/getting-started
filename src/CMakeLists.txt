# Application code shared by every platform, in two libraries:
#
#   ${APP_NAME}_core       the scenes, the shared context and the frame
#                          helpers. It depends on guillaume only: no engine,
#                          no Python.
#   ${APP_NAME}_scripting  the adapter between a host and Python. The only
#                          code that includes pybind11. It is a library of its
#                          own so the tests can exercise the Python bindings
#                          without opening a window.
#
# The desktop build (root CMakeLists.txt) and the Android build
# (platforms/android/app/cpp/CMakeLists.txt) both add this directory.

add_library(${APP_NAME}_core STATIC
    AppContext.cpp
    Frame.cpp
    scenes/HomeScene.cpp
)

target_include_directories(${APP_NAME}_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${APP_NAME}_core
    PUBLIC
        guillaume
)

add_library(${APP_NAME}_scripting STATIC
    scripting/Scripting.cpp
)

target_include_directories(${APP_NAME}_scripting
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${APP_NAME}_scripting
    PUBLIC
        mais
    PRIVATE
        pybind11::embed
)

if(APP_DEV_MODE)
    if(MSVC)
        target_compile_options(${APP_NAME}_core PRIVATE /W4 /WX)
    else()
        target_compile_options(${APP_NAME}_core PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
        )
    endif()
endif()
