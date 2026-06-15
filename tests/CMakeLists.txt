# ====================================================================
# Vix.cpp - JSON module tests
# ====================================================================

set(VIX_JSON_TEST_TARGETS
  json_build_test
  json_convert_test
  json_dumps_test
  json_include_test
  json_jpath_test
  json_loads_test
  json_simple_test
)

foreach(test_name IN LISTS VIX_JSON_TEST_TARGETS)
  add_executable(${test_name}
    ${test_name}.cpp
  )

  target_link_libraries(${test_name}
    PRIVATE
      vix::json
  )

  target_compile_features(${test_name}
    PRIVATE
      cxx_std_20
  )

  add_test(
    NAME ${test_name}
    COMMAND ${test_name}
  )
endforeach()

add_custom_target(vix_json_tests
  DEPENDS
    ${VIX_JSON_TEST_TARGETS}
)
