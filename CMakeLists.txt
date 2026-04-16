# ====================================================================
#  @file CMakeLists.txt
#  @author Gaspard Kirira
#
#  Copyright 2025, Gaspard Kirira. All rights reserved.
#  https://github.com/vixcpp/vix
#  Use of this source code is governed by a MIT license
#  that can be found in the License file.
#
# ====================================================================
# Vix.cpp - Json Module
# ====================================================================
# Purpose:
#   Provides the JSON abstraction layer for Vix using nlohmann_json.
#   Builds as a header-only INTERFACE target and can reuse an installed
#   provider or fetch a fallback header-only copy for local builds.
#
# Public Targets:
#   - vix_json   : The actual interface library target
#   - vix::json  : Namespaced alias for consumers
#
# Public Dependencies:
#   - nlohmann_json
#
# Installation/Export:
#   - Umbrella build exports into: VixTargets
#   - Standalone build exports into: vix_jsonTargets
# ====================================================================

cmake_minimum_required(VERSION 3.20)

project(vix_json VERSION 1.4.1 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(FetchContent)

# --------------------------------------------------------------------
# Global settings
# --------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --------------------------------------------------------------------
# Export set selection
# --------------------------------------------------------------------
if (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD)
  set(VIX_JSON_EXPORT_SET VixTargets)
else()
  set(VIX_JSON_EXPORT_SET vix_jsonTargets)
endif()

# --------------------------------------------------------------------
# Target
# --------------------------------------------------------------------
add_library(vix_json INTERFACE)
add_library(vix::json ALIAS vix_json)

set_target_properties(vix_json PROPERTIES
  EXPORT_NAME json
)

target_compile_features(vix_json INTERFACE cxx_std_20)

target_include_directories(vix_json INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set(_VIX_JSON_PROVIDER "none")
set(_VIX_JSON_INSTALL_FALLBACK OFF)

# --------------------------------------------------------------------
# Provider resolution
# Resolution policy:
#   1) Try installed CONFIG package
#   2) Try Find-module mode
#   3) Fallback to fetched header-only copy for local build
# --------------------------------------------------------------------
find_package(nlohmann_json QUIET CONFIG)

if (nlohmann_json_FOUND AND TARGET nlohmann_json::nlohmann_json)
  target_link_libraries(vix_json INTERFACE nlohmann_json::nlohmann_json)
  set(_VIX_JSON_PROVIDER "nlohmann_json (CONFIG)")

else()
  find_package(nlohmann_json QUIET MODULE)

  if (nlohmann_json_FOUND AND TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(vix_json INTERFACE nlohmann_json::nlohmann_json)
    set(_VIX_JSON_PROVIDER "nlohmann_json (MODULE)")

  else()
    message(STATUS "[json] nlohmann_json not found, fetching header-only fallback")

    FetchContent_Declare(
      nlohmann_json_fallback
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG        v3.11.3
      GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json_fallback)

    # Build-time fallback only. Do not export fetched target itself.
    target_include_directories(vix_json INTERFACE
      $<BUILD_INTERFACE:${nlohmann_json_fallback_SOURCE_DIR}/single_include>
    )

    set(_VIX_JSON_PROVIDER "nlohmann_json (fetched header-only)")
    set(_VIX_JSON_INSTALL_FALLBACK ON)
  endif()
endif()

# --------------------------------------------------------------------
# Examples
# --------------------------------------------------------------------
option(VIX_JSON_BUILD_EXAMPLES "Build json examples" OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (VIX_JSON_BUILD_EXAMPLES)
  add_executable(vix_json_quick examples/quick_start.cpp)
  target_link_libraries(vix_json_quick PRIVATE vix::json)

  add_executable(vix_json_builders examples/builders.cpp)
  target_link_libraries(vix_json_builders PRIVATE vix::json)

  add_executable(vix_json_jpath examples/jpath.cpp)
  target_link_libraries(vix_json_jpath PRIVATE vix::json)

  add_executable(vix_json_io examples/io.cpp)
  target_link_libraries(vix_json_io PRIVATE vix::json)

  add_custom_target(vix_json_examples
    DEPENDS
      vix_json_quick
      vix_json_builders
      vix_json_jpath
      vix_json_io
  )
endif()

# --------------------------------------------------------------------
# Install headers (standalone only)
# --------------------------------------------------------------------
if (NOT (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD))
  install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
  )

  if (_VIX_JSON_INSTALL_FALLBACK)
    install(
      DIRECTORY "${nlohmann_json_fallback_SOURCE_DIR}/single_include/nlohmann"
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
      FILES_MATCHING PATTERN "*.hpp"
    )
  endif()
endif()

# --------------------------------------------------------------------
# Install target
# --------------------------------------------------------------------
install(TARGETS vix_json
  EXPORT ${VIX_JSON_EXPORT_SET}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# --------------------------------------------------------------------
# Standalone package config/export
# Only for non-umbrella builds
# --------------------------------------------------------------------
if (NOT (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD))

  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/vix_jsonConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/vix_json"
  )

  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/vix_json"
  )

  install(EXPORT vix_jsonTargets
    FILE vix_jsonTargets.cmake
    NAMESPACE vix::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/vix_json"
  )

endif()

# --------------------------------------------------------------------
# Summary
# --------------------------------------------------------------------
message(STATUS "------------------------------------------------------")
message(STATUS "vix::json configured (${PROJECT_VERSION})")
message(STATUS "Export set:       ${VIX_JSON_EXPORT_SET}")
message(STATUS "Provider:         ${_VIX_JSON_PROVIDER}")
message(STATUS "Examples:         ${VIX_JSON_BUILD_EXAMPLES}")
message(STATUS "Install dir:      ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "------------------------------------------------------")
