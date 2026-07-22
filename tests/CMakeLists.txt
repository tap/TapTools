# Kernel unit tests — Catch2, pure C++, no Max / min-api / mock kernel. These run standalone
# (and in CI) via `ctest --test-dir <build>`; wrapper-level tests (attribute defaults, Min
# plumbing) stay with the externals under source/projects/ on the min-api harness.

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.0
    FIND_PACKAGE_ARGS 3
)
FetchContent_MakeAvailable(Catch2)

add_executable(taptools_kernel_tests
    autowah_test.cpp
    diode_ladder_test.cpp
    grm_comb_test.cpp
    nr_test.cpp
    spectra_test.cpp
    step_seq_test.cpp
    tune_test.cpp
    tr808_clap_test.cpp
    tr808_cymbal_test.cpp
    tr808_hat_test.cpp
    tr808_cowbell_test.cpp
    tr808_kick_test.cpp
    tr808_rim_test.cpp
    tr808_tom_test.cpp
    tr808_snare_test.cpp
    tb303_voice_test.cpp
    vca_test.cpp
    vco_test.cpp
    vocoder_test.cpp
    conv_engine_test.cpp
)
target_link_libraries(taptools_kernel_tests PRIVATE TapTools::taptools Catch2::Catch2WithMain)
set_target_properties(taptools_kernel_tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

add_test(NAME taptools_kernel_tests COMMAND taptools_kernel_tests)
