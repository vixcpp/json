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

# ---------------- Target (header-only) ----------------
add_library(vix_json INTERFACE)
add_library(Vix::json ALIAS vix_json)

target_compile_features(vix_json INTERFACE cxx_std_20)

# Public includes: allow #include <vix/json/Json.hpp>
target_include_directories(vix_json INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# ---------------- Dependencies ----------------
# nlohmann_json provides <nlohmann/json.hpp>
# Prefer CONFIG mode (modern package); fall back to MODULE if needed.
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(vix_json INTERFACE nlohmann_json::nlohmann_json)

# ---------------- Examples (opt-in) ----------------
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

# ---------------- Install / Export ----------------
# Install headers
install(DIRECTORY include/ DESTINATION include)

# Export interface target so downstream can do: find_package(vix_json)
install(TARGETS vix_json
  EXPORT vix_jsonTargets
)

# Export the targets file (for build tree and install tree)
export(EXPORT vix_jsonTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonTargets.cmake"
  NAMESPACE Vix::
)

# Create a simple Config + Version files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Minimal config just re-includes exported targets
# (no template file needed)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonConfig.cmake"
"@PACKAGE_INIT@
include(\"\${CMAKE_CURRENT_LIST_DIR}/vix_jsonTargets.cmake\")
")

# Install config files
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonConfigVersion.cmake"
  DESTINATION lib/cmake/vix_json
)

# Install the exported targets
install(EXPORT vix_jsonTargets
  FILE vix_jsonTargets.cmake
  NAMESPACE Vix::
  DESTINATION lib/cmake/vix_json
)

# ---------------- Status ----------------
message(STATUS "vix_json header-only module configured (version ${PROJECT_VERSION})")
