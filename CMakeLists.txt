cmake_minimum_required(VERSION 3.20)
project(vix_json LANGUAGES CXX)

add_library(vix_json INTERFACE)
target_compile_features(vix_json INTERFACE cxx_std_20)

target_include_directories(vix_json INTERFACE include)

add_library(vix::json ALIAS vix_json)

option(VIX_JSON_BUILD_EXAMPLES "Build json examples" ON)
if (VIX_JSON_BUILD_EXAMPLES)
  add_executable(vix_json_quick examples/quick_start.cpp)
  target_link_libraries(vix_json_quick PRIVATE vix::json)
  target_include_directories(vix_json_quick PRIVATE include)

  add_executable(vix_json_builders examples/builders.cpp)
  target_link_libraries(vix_json_builders PRIVATE vix::json)
  target_include_directories(vix_json_builders PRIVATE include)

  add_executable(vix_json_jpath examples/jpath.cpp)
  target_link_libraries(vix_json_jpath PRIVATE vix::json)
  target_include_directories(vix_json_jpath PRIVATE include)

  add_executable(vix_json_io examples/io.cpp)
  target_link_libraries(vix_json_io PRIVATE vix::json)
  target_include_directories(vix_json_io PRIVATE include)
endif()
