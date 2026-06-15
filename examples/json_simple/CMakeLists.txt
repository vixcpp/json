# ====================================================================
# Vix.cpp - JSON Simple examples
# ====================================================================

set(VIX_JSON_SIMPLE_EXAMPLE_TARGETS
  vix_json_simple_01_basic_values
  vix_json_simple_02_arrays
  vix_json_simple_03_objects
  vix_json_simple_04_nested
  vix_json_simple_05_mutation
  vix_json_simple_06_iteration
  vix_json_simple_07_merge_and_erase
  vix_json_simple_08_to_nlohmann_json
  vix_json_simple_bench_simple_vs_nlohmann
)

add_executable(vix_json_simple_01_basic_values
  01_basic_values.cpp
)

add_executable(vix_json_simple_02_arrays
  02_arrays.cpp
)

add_executable(vix_json_simple_03_objects
  03_objects.cpp
)

add_executable(vix_json_simple_04_nested
  04_nested.cpp
)

add_executable(vix_json_simple_05_mutation
  05_mutation.cpp
)

add_executable(vix_json_simple_06_iteration
  06_iteration.cpp
)

add_executable(vix_json_simple_07_merge_and_erase
  07_merge_and_erase.cpp
)

add_executable(vix_json_simple_08_to_nlohmann_json
  08_to_nlohmann_json.cpp
)

add_executable(vix_json_simple_bench_simple_vs_nlohmann
  bench_simple_vs_nlohmann.cpp
)

foreach(example_target IN LISTS VIX_JSON_SIMPLE_EXAMPLE_TARGETS)
  target_link_libraries(${example_target}
    PRIVATE
      vix::json
  )

  target_compile_features(${example_target}
    PRIVATE
      cxx_std_20
  )
endforeach()

add_custom_target(vix_json_simple_examples
  DEPENDS
    ${VIX_JSON_SIMPLE_EXAMPLE_TARGETS}
)
