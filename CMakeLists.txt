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
#   - vix::json      (compat alias, optional)
# ====================================================================

cmake_minimum_required(VERSION 3.20)
project(vix_json VERSION 0.2.0 LANGUAGES CXX)

# ---------------- Target (header-only) ----------------
add_library(vix_json INTERFACE)
add_library(Vix::json ALIAS vix_json)
# Compat alias (certains projets attendent vix::json)
add_library(vix::json ALIAS vix_json)

target_compile_features(vix_json INTERFACE cxx_std_20)

# Public includes: allow #include <vix/json/...>
target_include_directories(vix_json INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# ---------------- Dependencies ----------------
# Essaye d'abord le paquet moderne nlohmann_json (CONFIG), puis MODULE.
# En dernier recours, FetchContent la version header-only officielle.
set(_have_nlohmann OFF)

find_package(nlohmann_json QUIET CONFIG)
if (nlohmann_json_FOUND)
  target_link_libraries(vix_json INTERFACE nlohmann_json::nlohmann_json)
  set(_have_nlohmann ON)
else()
  find_package(nlohmann_json QUIET MODULE)
  if (nlohmann_json_FOUND)
    target_link_libraries(vix_json INTERFACE nlohmann_json::nlohmann_json)
    set(_have_nlohmann ON)
  endif()
endif()

if (NOT _have_nlohmann)
  message(STATUS "[json] nlohmann_json not found — fetching header-only fallback")

  include(FetchContent)
  FetchContent_Declare(
    nlohmann_json_fallback
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    # Utilise la lib header-only (pas de build)
  )
  FetchContent_MakeAvailable(nlohmann_json_fallback)

  # Le repo fournit <nlohmann/json.hpp> sous ${nlohmann_json_SOURCE_DIR}/single_include
  # On ajoute ce chemin aux consommateurs via INTERFACE.
  target_include_directories(vix_json INTERFACE
    $<BUILD_INTERFACE:${nlohmann_json_SOURCE_DIR}/single_include>)

  # Optionnel : définir une interface IMPORTED si besoin (compat)
  add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
  target_include_directories(nlohmann_json::nlohmann_json INTERFACE
    $<BUILD_INTERFACE:${nlohmann_json_SOURCE_DIR}/single_include>)
  target_link_libraries(vix_json INTERFACE nlohmann_json::nlohmann_json)
endif()

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
install(DIRECTORY include/ DESTINATION include)

install(TARGETS vix_json
  EXPORT vix_jsonTargets
)

export(EXPORT vix_jsonTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonTargets.cmake"
  NAMESPACE Vix::
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Config minimal qui réimporte les targets exportés
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonConfig.cmake"
"@PACKAGE_INIT@
include(\"\${CMAKE_CURRENT_LIST_DIR}/vix_jsonTargets.cmake\")
")

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/vix_jsonConfigVersion.cmake"
  DESTINATION lib/cmake/vix_json
)

install(EXPORT vix_jsonTargets
  FILE vix_jsonTargets.cmake
  NAMESPACE Vix::
  DESTINATION lib/cmake/vix_json
)

# ---------------- Status ----------------
message(STATUS "vix_json header-only module configured (version ${PROJECT_VERSION})")
message(STATUS "nlohmann_json provider: ${nlohmann_json_FOUND}")
