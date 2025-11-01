cmake_minimum_required(VERSION 3.20)

# ====================================================================
# Vix.cpp — JSON Module (Header-only, Umbrella-integrated)
# ====================================================================
# Purpose
#   Lightweight JSON facade/adapters used by Vix core and user apps.
#   The module is header-only to keep build overhead minimal.
#
# Targets
#   - vix_json   : Real header-only target
#   - vix::json  : Preferred namespaced alias for consumers
#   - vix::json  : Compatibility alias (some projects expect this)
#
# Integration & Install
#   - Public headers live under modules/json/include/...
#   - Prefer system-provided nlohmann_json; otherwise fetch header-only.
#   - Contributes to the umbrella export-set `VixTargets` so that
#     a system-wide Vix install exposes vix::json via:
#       find_package(Vix CONFIG REQUIRED)
#       target_link_libraries(app PRIVATE vix::json)
# ====================================================================

project(vix_json VERSION 1.3.1 LANGUAGES CXX)

include(GNUInstallDirs)
include(FetchContent)

# ---------------- Target (header-only) ----------------
add_library(vix_json INTERFACE)
add_library(vix::json ALIAS vix_json)
add_library(Vix::json ALIAS vix_json)

target_compile_features(vix_json INTERFACE cxx_std_20)

# Public include layout to allow: #include <vix/json/...>
target_include_directories(vix_json INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ---------------- Dependency: nlohmann_json ----------------
set(_VIX_JSON_PROVIDER "none")

find_package(nlohmann_json QUIET CONFIG)
if (nlohmann_json_FOUND)
  target_link_libraries(vix_json INTERFACE nlohmann_json::nlohmann_json)
  set(_VIX_JSON_PROVIDER "nlohmann_json (CONFIG)")
else()
  find_package(nlohmann_json QUIET MODULE)
  if (nlohmann_json_FOUND)
    target_link_libraries(vix_json INTERFACE nlohmann_json::nlohmann_json)
    set(_VIX_JSON_PROVIDER "nlohmann_json (MODULE)")
  else()
    message(STATUS "[json] nlohmann_json not found — fetching header-only fallback")
    FetchContent_Declare(
      nlohmann_json_fallback
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG        v3.11.3
      GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json_fallback)

    target_include_directories(vix_json INTERFACE
      $<BUILD_INTERFACE:${nlohmann_json_SOURCE_DIR}/single_include>)

    if (NOT TARGET nlohmann_json::nlohmann_json)
      add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
      target_include_directories(nlohmann_json::nlohmann_json INTERFACE
        $<BUILD_INTERFACE:${nlohmann_json_SOURCE_DIR}/single_include>)
    endif()

    target_link_libraries(vix_json INTERFACE nlohmann_json::nlohmann_json)
    set(_VIX_JSON_PROVIDER "nlohmann_json (fetched header-only)")
  endif()
endif()

# ---------------- Examples (opt-in) ----------------
option(VIX_JSON_BUILD_EXAMPLES "Build json examples" OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (VIX_JSON_BUILD_EXAMPLES)
  add_executable(vix_json_quick     examples/quick_start.cpp)
  target_link_libraries(vix_json_quick PRIVATE vix::json)

  add_executable(vix_json_builders  examples/builders.cpp)
  target_link_libraries(vix_json_builders PRIVATE vix::json)

  add_executable(vix_json_jpath     examples/jpath.cpp)
  target_link_libraries(vix_json_jpath PRIVATE vix::json)

  add_executable(vix_json_io        examples/io.cpp)
  target_link_libraries(vix_json_io PRIVATE vix::json)

  add_custom_target(vix_json_examples
    DEPENDS vix_json_quick vix_json_builders vix_json_jpath vix_json_io
  )
endif()

# ---------------- Install / Export (umbrella) ----------------
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

install(TARGETS vix_json
  EXPORT  VixTargets                 # <- join umbrella export-set
)

# ---------------- Status ----------------
message(STATUS "------------------------------------------------------")
message(STATUS "vix_json configured (version ${PROJECT_VERSION})")
message(STATUS "Provider: ${_VIX_JSON_PROVIDER}")
message(STATUS "Examples: ${VIX_JSON_BUILD_EXAMPLES}")
message(STATUS "Install include dir: ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "------------------------------------------------------")

