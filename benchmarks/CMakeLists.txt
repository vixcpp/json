# ====================================================================
# Vix.cpp - JSON module benchmarks
# ====================================================================

set(VIX_JSON_BENCHMARK_TARGETS
  json_build_bench
  json_convert_bench
  json_dumps_bench
  json_jpath_bench
  json_loads_bench
  json_simple_bench
)

foreach(bench_name IN LISTS VIX_JSON_BENCHMARK_TARGETS)
  add_executable(${bench_name}
    ${bench_name}.cpp
  )

  target_link_libraries(${bench_name}
    PRIVATE
      vix::json
  )

  target_compile_features(${bench_name}
    PRIVATE
      cxx_std_20
  )
endforeach()

add_custom_target(vix_json_benchmarks
  DEPENDS
    ${VIX_JSON_BENCHMARK_TARGETS}
)
