# ======================================================
# vix.cpp json module (header-only)
# ======================================================
cmake_minimum_required(VERSION 3.20)
project(vix_json VERSION 0.2.0 LANGUAGES CXX)

add_library(vix_json INTERFACE)
add_library(Vix::json ALIAS vix_json)

target_compile_features(vix_json INTERFACE cxx_std_20)
target_include_directories(vix_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Optionnel: propager nlohmann_json si find_package() le trouve
# find_package(nlohmann_json QUIET)
# if(nlohmann_json_FOUND)
#   target_link_libraries(vix_json INTERFACE nlohmann_json::nlohmann_json)
# endif()

option(VIX_JSON_BUILD_EXAMPLES "Build json examples" ON)
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

# Install/export (optionnel)
option(VIX_JSON_INSTALL "Install vix_json headers and export targets" OFF)
if (VIX_JSON_INSTALL)
  install(TARGETS vix_json EXPORT vix_jsonTargets)
  install(DIRECTORY include/ DESTINATION include)
  install(EXPORT vix_jsonTargets
    NAMESPACE Vix::
    FILE vix_jsonTargets.cmake
    DESTINATION lib/cmake/vix_json)
endif()
