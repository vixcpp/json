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

set(VIX_JSON_HEADER_TESTS
  build
  convert
  dumps
  jpath
  json
  loads
  simple
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

  if (VIX_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(${test_name}
      PRIVATE
        -fno-omit-frame-pointer
        -fsanitize=address,undefined
    )

    target_link_options(${test_name}
      PRIVATE
        -fsanitize=address,undefined
    )
  endif()

  add_test(
    NAME ${test_name}
    COMMAND ${test_name}
  )
endforeach()

foreach(header_name IN LISTS VIX_JSON_HEADER_TESTS)
  set(test_name json_${header_name}_header_test)

  add_executable(${test_name}
    headers/${header_name}_header_test.cpp
  )

  target_link_libraries(${test_name}
    PRIVATE
      vix::json
  )

  target_compile_features(${test_name}
    PRIVATE
      cxx_std_20
  )

  if (TARGET vix_warnings)
    target_link_libraries(${test_name}
      PRIVATE
        vix_warnings
    )
  elseif (MSVC)
    target_compile_options(${test_name}
      PRIVATE
        /W4
        /permissive-
    )
  else()
    target_compile_options(${test_name}
      PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
    )
  endif()

  add_test(
    NAME ${test_name}
    COMMAND ${test_name}
  )

  list(APPEND VIX_JSON_TEST_TARGETS ${test_name})
endforeach()

set(literal_test_name json_literals_header_test)

add_executable(${literal_test_name}
  headers/json_header_test.cpp
)

target_link_libraries(${literal_test_name}
  PRIVATE
    vix::json
)

target_compile_features(${literal_test_name}
  PRIVATE
    cxx_std_20
)

target_compile_definitions(${literal_test_name}
  PRIVATE
    VIX_JSON_ENABLE_LITERALS=1
)

if (TARGET vix_warnings)
  target_link_libraries(${literal_test_name}
    PRIVATE
      vix_warnings
  )
elseif (MSVC)
  target_compile_options(${literal_test_name}
    PRIVATE
      /W4
      /permissive-
  )
else()
  target_compile_options(${literal_test_name}
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wsign-conversion
  )
endif()

add_test(
  NAME ${literal_test_name}
  COMMAND ${literal_test_name}
)

list(APPEND VIX_JSON_TEST_TARGETS ${literal_test_name})

add_custom_target(vix_json_tests
  DEPENDS
    ${VIX_JSON_TEST_TARGETS}
)
