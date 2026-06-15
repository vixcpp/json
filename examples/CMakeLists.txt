# ====================================================================
# Vix.cpp - JSON module examples
# ====================================================================

set(VIX_JSON_EXAMPLE_TARGETS
  vix_json_quick
  vix_json_builders
  vix_json_jpath
  vix_json_io
)

add_executable(vix_json_quick
  quick_start.cpp
)

add_executable(vix_json_builders
  builders.cpp
)

add_executable(vix_json_jpath
  jpath.cpp
)

add_executable(vix_json_io
  io.cpp
)

foreach(example_target IN LISTS VIX_JSON_EXAMPLE_TARGETS)
  target_link_libraries(${example_target}
    PRIVATE
      vix::json
  )

  target_compile_features(${example_target}
    PRIVATE
      cxx_std_20
  )
endforeach()

add_subdirectory(json_simple)

add_custom_target(vix_json_examples
  DEPENDS
    ${VIX_JSON_EXAMPLE_TARGETS}
    vix_json_simple_examples
)
