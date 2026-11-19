# Copies a directory into the APK assets together with a manifest.
#
# The Android asset manager cannot list sub-directories from native code, and
# Python needs real files: at start-up the Android host reads the manifest and
# copies the tree to the application's private directory (see
# platforms/android/app/cpp/AssetTree.hpp).
#
# Run as a script, on every build:
#
#   cmake -DSOURCE_DIR=<dir> -DOUTPUT_DIR=<dir> [-DEXCLUDE=<regex>;<regex>...]
#         [-DSTAMP=<text>] -P AssetTree.cmake
#
# SOURCE_DIR  the tree to copy
# OUTPUT_DIR  where it is copied; replaced when its content is out of date
# EXCLUDE     a list of regular expressions: a file is left out when its path
#             below SOURCE_DIR matches one of them
# STAMP       identifies the content. When it is not given, it is computed
#             from the content of every file, which suits a small tree that
#             changes often; give it for a large tree that never changes.
#
# OUTPUT_DIR/manifest.txt holds the stamp on its first line, then one relative
# path per line. The host extracts the tree again when the stamp changes.

cmake_minimum_required(VERSION 3.28)

foreach(app_required SOURCE_DIR OUTPUT_DIR)
    if(NOT ${app_required})
        message(FATAL_ERROR "AssetTree.cmake: ${app_required} is not set")
    endif()
endforeach()

file(GLOB_RECURSE app_candidates
    LIST_DIRECTORIES false
    RELATIVE ${SOURCE_DIR}
    ${SOURCE_DIR}/*
)
list(SORT app_candidates)

set(app_files "")
foreach(app_file IN LISTS app_candidates)
    set(app_excluded FALSE)
    foreach(app_pattern IN LISTS EXCLUDE)
        if(app_file MATCHES "${app_pattern}")
            set(app_excluded TRUE)
            break()
        endif()
    endforeach()
    if(NOT app_excluded)
        list(APPEND app_files "${app_file}")
    endif()
endforeach()

if(NOT STAMP)
    set(app_digest "")
    foreach(app_file IN LISTS app_files)
        file(SHA256 ${SOURCE_DIR}/${app_file} app_hash)
        string(APPEND app_digest "${app_file}:${app_hash}\n")
    endforeach()
    string(SHA256 STAMP "${app_digest}")
endif()

set(app_manifest ${OUTPUT_DIR}/manifest.txt)

if(EXISTS ${app_manifest})
    file(STRINGS ${app_manifest} app_current_stamp LIMIT_COUNT 1)
    if(app_current_stamp STREQUAL STAMP)
        return()
    endif()
endif()

file(REMOVE_RECURSE ${OUTPUT_DIR})

set(app_listing "${STAMP}\n")
foreach(app_file IN LISTS app_files)
    cmake_path(GET app_file PARENT_PATH app_parent)
    file(COPY ${SOURCE_DIR}/${app_file} DESTINATION ${OUTPUT_DIR}/${app_parent})
    string(APPEND app_listing "${app_file}\n")
endforeach()

# Written last: an interrupted copy leaves no manifest and is done again.
file(WRITE ${app_manifest} "${app_listing}")

list(LENGTH app_files app_count)
message(STATUS "Copied ${app_count} files to ${OUTPUT_DIR}")
