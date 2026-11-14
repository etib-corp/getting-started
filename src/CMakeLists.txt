# Application code shared by every platform: the scenes, the shared context
# and the frame helpers. It depends on guillaume only: no engine, no Python.
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
