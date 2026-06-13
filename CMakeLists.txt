# Copyright 2002-2026 Timothy Place. Distributed under the New BSD License.
#
# Modern CMake build for TapTools, based on the Min-DevKit (min-api + max-sdk-base).
# This replaces the retired Ruby/Xcode-project + Travis build. Objects live in
# source/projects/<name>/ and are built as Max externals (.mxo / .mxe64).

cmake_minimum_required(VERSION 3.19)
project(TapTools)

enable_testing()

string(REGEX REPLACE "(.*)/" "" THIS_PACKAGE_NAME "${CMAKE_CURRENT_SOURCE_DIR}")

if (APPLE)
    # Build universal binaries (Apple Silicon + Intel) by default. The existing 2015-era
    # binaries were i386/x86_64 only and will not run on Apple Silicon; never repeat that.
    if (NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures" FORCE)
        message(STATUS "CMAKE_OSX_ARCHITECTURES set to ${CMAKE_OSX_ARCHITECTURES}")
    endif ()
endif ()

# Min-DevKit package helpers (defines SUBDIRLIST, packaging, etc.)
include(${CMAKE_CURRENT_SOURCE_DIR}/source/min-api/script/min-package.cmake)

# Add Min-Lib if present (used by DSP objects as the revival progresses).
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/min-lib/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/min-lib)
endif ()

# Generate a build target for every object folder in source/projects.
SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source/projects)
foreach (project_dir ${PROJECT_DIRS})
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir}/CMakeLists.txt")
        message(STATUS "Generating: ${project_dir}")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir})
    endif ()
endforeach ()
