# ====================================================================
# Vix.cpp — JSON Module (Header-only)
# ====================================================================
# Purpose:
#   - Lightweight JSON facade / adapters.
#   - Header-only for minimal build overhead.
#
# Targets:
#   - vix_json       (real target)
#   - Vix::json      (alias, preferred by consumers)
#
# Usage:
#   target_link_libraries(your_target PRIVATE/INTERFACE Vix::json)
# ====================================================================

cmake_minimum_required(VERSION 3.20)
project(vix_json VERSION 0.2.0 LANGUAGES CXX)

add_library(vix_json INTERFACE)
add_library(Vix::json ALIAS vix_json)

target_compile_features(vix_json INTERFACE cxx_std_20)

# Expose public headers so users can do #include <vix/...>
target_include_directories(vix_json INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Examples (opt-in; disabled by default for faster CI)
option(VIX_JSON_BUILD_EXAMPLES "Build json examples" OFF)
if (VIX_JSON_BUILD_EXAMPLES)
  add_executable(vix_json_quick     examples/quick_start.cpp)
  target_link_libraries(vix_json_quick PRIVATE Vix::json)

  add_executable(vix_json_builders  examples/builders.cpp)
  target_link_libraries(vix_json_builders PRIVATE Vix::json)

  add_executable(vix_json_jpath     examples/jpath.cpp)
  target_link_libraries(vix_json_jpath PRIVATE Vix::json)

  add_executable(vix_json_io        examples/io.cpp)
  target_link_libraries(vix_json_io PRIVATE Vix::json)
endif()
