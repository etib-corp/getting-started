# Python for the Android build.
#
# The desktop build embeds the Python installed on the machine. There is no
# such thing when cross-compiling for a headset, so the Android build uses the
# official "Android embeddable package" published on python.org: a prebuilt
# libpython, its headers and its standard library, one archive per ABI.
#
#   https://www.python.org/downloads/release/python-3152/
#
# This file is included by Dependencies.cmake before mais is fetched. It:
#   1. downloads the archive of the target ABI and checks its SHA-256;
#   2. points CMake's FindPython (used by pybind11, used by mais) at it;
#   3. defines the variables the Android build uses to package the runtime
#      (see platforms/android/app/cpp/CMakeLists.txt).
#
# To move to another Python release, change the version and the checksums
# below (they are in the "Files" table of the release page).

include(FetchContent)

set(APP_PYTHON_ANDROID_VERSION 3.15.2)

set(APP_PYTHON_ANDROID_TRIPLET_arm64-v8a aarch64-linux-android)
set(APP_PYTHON_ANDROID_SHA256_arm64-v8a
    cfd4f0fdaff465ba62ebf3c3df2c8c410d0802fef3f98897082460df704cfef6
)
set(APP_PYTHON_ANDROID_TRIPLET_x86_64 x86_64-linux-android)
set(APP_PYTHON_ANDROID_SHA256_x86_64
    e5b46c72295e558991bbe98415ac29bc1448aca35a8e660ddfe017e20305cbf8
)

if(NOT DEFINED APP_PYTHON_ANDROID_TRIPLET_${ANDROID_ABI})
    message(FATAL_ERROR
        "python.org publishes no Android package for the ABI "
        "'${ANDROID_ABI}' (supported: arm64-v8a, x86_64)."
    )
endif()

set(APP_PYTHON_ANDROID_TRIPLET ${APP_PYTHON_ANDROID_TRIPLET_${ANDROID_ABI}})
set(APP_PYTHON_ANDROID_SHA256 ${APP_PYTHON_ANDROID_SHA256_${ANDROID_ABI}})

string(REGEX MATCH "^[0-9]+\\.[0-9]+" APP_PYTHON_ANDROID_SERIES
    "${APP_PYTHON_ANDROID_VERSION}"
)

# 01. The package
#
# It has no CMakeLists.txt at its root, so it is only downloaded and
# extracted. One directory per ABI: Gradle configures each ABI separately.

FetchContent_Declare(python_android_${ANDROID_ABI}
    URL https://www.python.org/ftp/python/${APP_PYTHON_ANDROID_VERSION}/python-${APP_PYTHON_ANDROID_VERSION}-${APP_PYTHON_ANDROID_TRIPLET}.tar.gz
    URL_HASH SHA256=${APP_PYTHON_ANDROID_SHA256}
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)
FetchContent_MakeAvailable(python_android_${ANDROID_ABI})

string(TOLOWER python_android_${ANDROID_ABI} app_python_android_name)
set(APP_PYTHON_ANDROID_PREFIX ${${app_python_android_name}_SOURCE_DIR}/prefix)

# The standard library, relative to the Python home (the prefix here, the
# application's private directory on the headset).
set(APP_PYTHON_ANDROID_STDLIB_SUBDIR lib/python${APP_PYTHON_ANDROID_SERIES})
set(APP_PYTHON_ANDROID_STDLIB_DIR
    ${APP_PYTHON_ANDROID_PREFIX}/${APP_PYTHON_ANDROID_STDLIB_SUBDIR}
)

# libpython is linked by the application, so Gradle packages it by itself.
# These are loaded by the extension modules of the standard library (ssl,
# hashlib, sqlite3) and have to be packaged explicitly.
set(APP_PYTHON_ANDROID_RUNTIME_LIBRARIES
    ${APP_PYTHON_ANDROID_PREFIX}/lib/libcrypto_python.so
    ${APP_PYTHON_ANDROID_PREFIX}/lib/libssl_python.so
    ${APP_PYTHON_ANDROID_PREFIX}/lib/libsqlite3_python.so
)

# What is left out of the APK: the test suite, the desktop GUI and packaging
# tools, the build configuration, and the extension modules that only exist
# for the test suite. Regular expressions on the path below the standard
# library directory.
set(APP_PYTHON_ANDROID_STDLIB_EXCLUDE
    "(^|/)__pycache__/"
    "^test/"
    "^idlelib/"
    "^tkinter/"
    "^turtledemo/"
    "^turtle\\.py$"
    "^ensurepip/"
    "^venv/"
    "^pydoc_data/"
    "^site-packages/"
    "^config-"
    "^lib-dynload/_test"
    "^lib-dynload/_ctypes_test"
    "^lib-dynload/_xxtestfuzz"
    "^lib-dynload/xx"
)

# 02. Python discovery
#
# FindPython cannot run the target interpreter, and the toolchain only
# searches the NDK sysroot: give it the two artifacts directly, and tell
# pybind11 not to look for an interpreter (it would find the build machine's
# Python and ask for development files of that version). mais does not
# document this yet: https://github.com/etib-corp/mais/issues/40

set(PYBIND11_USE_CROSSCOMPILING ON CACHE BOOL "" FORCE)

set(Python_INCLUDE_DIR
    ${APP_PYTHON_ANDROID_PREFIX}/include/python${APP_PYTHON_ANDROID_SERIES}
    CACHE PATH "Headers of the Python built for Android" FORCE
)
set(Python_LIBRARY
    ${APP_PYTHON_ANDROID_PREFIX}/lib/libpython${APP_PYTHON_ANDROID_SERIES}.so
    CACHE FILEPATH "The Python library built for Android" FORCE
)
