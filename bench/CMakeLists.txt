# CPU benchmarks (kernel-only, no Max dependency). Each bench times its kernel over a config
# matrix; the ratchet workflow (compare.py + baselines/) is described in README.md.

foreach (bench svf_bench diode_bench)
    add_executable(${bench} ${bench}.cpp)
    target_link_libraries(${bench} PRIVATE TapTools::taptools)
    set_target_properties(${bench} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
endforeach ()
