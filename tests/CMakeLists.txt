# Tests of the application.
#
# The scripting tests start Python but no window, so they run anywhere,
# including CI machines without a display or a GPU.

add_executable(${APP_NAME}_scripting_test
    ScriptingTest.cpp
)

target_link_libraries(${APP_NAME}_scripting_test PRIVATE
    ${APP_NAME}_scripting
)

# The application's real script: catches a syntax error or a hook that no
# longer matches the bindings.
add_test(NAME scripting.app_script
    COMMAND ${APP_NAME}_scripting_test ${CMAKE_SOURCE_DIR}/scripts app
)

# Fixture scripts for the behaviour of the host bindings.
add_test(NAME scripting.bindings
    COMMAND ${APP_NAME}_scripting_test
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts fixture
)

# The tests load python3XX.dll from the interpreter's own directory. PATH is
# prepended when CMake knows the interpreter, so a configure that selected it
# with -DPython_EXECUTABLE runs the tests too.
if(APP_PYTHON_DIR)
    set_tests_properties(scripting.app_script scripting.bindings PROPERTIES
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:${APP_PYTHON_DIR}"
    )
endif()

# Opens the window, draws 120 frames and exits. Off by default because it
# needs a display and a Vulkan driver: -DAPP_ENABLE_GPU_TESTS=ON.
if(APP_ENABLE_GPU_TESTS)
    add_test(NAME app.smoke
        COMMAND ${APP_NAME}
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${APP_NAME}>
    )
    set_tests_properties(app.smoke PROPERTIES
        ENVIRONMENT "GETTING_STARTED_MAX_FRAMES=120"
        PASS_REGULAR_EXPRESSION "app.py: stopped after 120 frames"
        TIMEOUT 120
    )
    if(APP_PYTHON_DIR)
        set_tests_properties(app.smoke PROPERTIES
            ENVIRONMENT_MODIFICATION
                "PATH=path_list_prepend:${APP_PYTHON_DIR}"
        )
    endif()
endif()
