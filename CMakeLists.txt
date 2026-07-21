# Copyright 2002-2026 Timothy Place. Distributed under the New BSD License.
#
# TapTools kernel — the portable DSP library behind the TapTools Max package.
# Header-only, plain C++20: no Max SDK, no min-api, no Jamoma. The kernels live
# under include/taptools/, one self-contained header per object, in the `taptools` namespace.
#
# This is a complete standalone CMake project (the AmbiTap / AmbiTap-Max pattern): it lives in
# kernel/ of the TapTools monorepo and is designed to lift verbatim into its own repository. The
# Max package's externals consume only include/ (via TAPTOOLS_KERNEL_DIR); the kernel's own
# tests, tools, and benchmarks build from this directory:
#
#     cmake -S . -B build -DCMAKE_BUILD_TYPE=Release   # or -S kernel from the monorepo root
#     cmake --build build
#     ctest --test-dir build

cmake_minimum_required(VERSION 3.19)
project(TapToolsKernel VERSION 0.1.0 LANGUAGES CXX)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(TAPTOOLS_KERNEL_TOP_LEVEL ON)
else ()
    set(TAPTOOLS_KERNEL_TOP_LEVEL OFF)
endif ()

option(TAPTOOLS_BUILD_TESTS "Build the kernel unit tests" ${TAPTOOLS_KERNEL_TOP_LEVEL})
option(TAPTOOLS_BUILD_TOOLS "Build the offline render tools and the C ABI" ${TAPTOOLS_KERNEL_TOP_LEVEL})
option(TAPTOOLS_BUILD_BENCH "Build the CPU benchmarks" ${TAPTOOLS_KERNEL_TOP_LEVEL})
option(TAPTOOLS_INSTALL "Generate install and package-export rules" ${TAPTOOLS_KERNEL_TOP_LEVEL})

# Shared real FFT (DspTap) — the Ooura wrapper used by the spectral kernels
# (conv_engine / stft / nr / spectra), formerly a private radix-2 copy in
# fft.h. Consumed from the tap/dsptap submodule as tap::dsp, which carries the
# split-radix Ooura transform plus the per-target vDSP / CMSIS-Helium float32
# backends. Pinned as submodules/dsptap (the AmbiTap / MuTap pattern).
add_subdirectory(submodules/dsptap)

add_library(taptools INTERFACE)
add_library(TapTools::taptools ALIAS taptools)
target_compile_features(taptools INTERFACE cxx_std_20)
target_include_directories(taptools INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
# tap::dsp brings the real-FFT header and links the Ooura static lib.
target_link_libraries(taptools INTERFACE tap::dsp)

if (TAPTOOLS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (TAPTOOLS_BUILD_TOOLS)
    add_subdirectory(tools/render)
    add_subdirectory(tools/capi)
endif ()

if (TAPTOOLS_BUILD_BENCH)
    add_subdirectory(bench)
endif ()

if (TAPTOOLS_INSTALL)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    # The DspTap real FFT is part of taptools' interface (the spectral kernels
    # include tap/dsp/fft.h), so its headers and targets join the export set.
    install(DIRECTORY submodules/dsptap/include/tap DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(TARGETS taptools tap_dsp tap_dsp_fft EXPORT TapToolsKernelTargets)
    install(EXPORT TapToolsKernelTargets
        NAMESPACE TapTools::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TapToolsKernel)
    install(FILES submodules/dsptap/third_party/ooura/readme.txt
        DESTINATION ${CMAKE_INSTALL_DATADIR}/taptools
        RENAME OOURA_LICENSE.txt)

    configure_package_config_file(cmake/TapToolsKernelConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/TapToolsKernelConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TapToolsKernel)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/TapToolsKernelConfigVersion.cmake
        COMPATIBILITY SameMajorVersion
        ARCH_INDEPENDENT)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/TapToolsKernelConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/TapToolsKernelConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TapToolsKernel)
endif ()
