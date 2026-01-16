/**
 *
 *  @file loads.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_LOADS_HPP
#define VIX_LOADS_HPP

#include <nlohmann/json.hpp>
#include <string_view>
#include <optional>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <string>

/**
 * @file VIX_LOADS_HPP
 * @brief JSON parsing utilities for strings and files in *Vix.cpp*.
 *
 * This header provides a set of safe and ergonomic helpers to parse JSON
 * from strings or files, with both **throwing** and **noexcept (optional)** versions.
 *
 * ### Overview
 * - `loads()` → parse from string, throws on error.
 * - `try_loads()` → parse from string, returns `std::nullopt` on error.
 * - `load_file()` → parse a JSON file, throws on error.
 * - `try_load_file()` → parse a JSON file, returns `std::nullopt` on error.
 *
 * All functions rely on *nlohmann/json* and integrate cleanly with the
 * rest of the Vix.cpp JSON API (`Build`, `Dumps`, `Convert`, etc.).
 *
 * ### Example
 * @code
 * using namespace vix::json;
 *
 * // Parse from string
 * auto j = loads(R"({"id": 1, "name": "Softadastra"})");
 * std::cout << j["name"] << std::endl; // "Softadastra"
 *
 * // Safe parsing
 * if (auto maybe = try_loads("not-json")) {
 *     std::cout << (*maybe).dump(2);
 * } else {
 *     std::cout << "Invalid JSON" << std::endl;
 * }
 *
 * // Parse from file
 * auto conf = load_file("config.json");
 *
 * // Optional version
 * if (auto data = try_load_file("settings.json")) {
 *     std::cout << "Loaded " << data->size() << " entries" << std::endl;
 * }
 * @endcode
 */

namespace vix::json
{
  using Json = nlohmann::json;
  namespace fs = std::filesystem;

  /**
   * @brief Parse a JSON document from a `std::string_view`.
   *
   * This function throws if the input is not valid JSON.
   *
   * @param s UTF-8 JSON text.
   * @return Parsed JSON value.
   * @throws nlohmann::json::parse_error if the content is invalid.
   */
  inline Json loads(std::string_view s)
  {
    return Json::parse(s);
  }

  /**
   * @brief Parse JSON from string, returning `std::optional` on error.
   *
   * @param s UTF-8 JSON text.
   * @return Parsed JSON or `std::nullopt` if parsing fails.
   */
  inline std::optional<Json> try_loads(std::string_view s) noexcept
  {
    try
    {
      return Json::parse(s);
    }
    catch (...)
    {
      return std::nullopt;
    }
  }

  /**
   * @brief Read and parse a JSON file into a `Json` object.
   *
   * This version throws on I/O or parse error.
   *
   * @param path Filesystem path to a `.json` file.
   * @return Parsed JSON document.
   * @throws std::runtime_error if the file cannot be opened.
   * @throws nlohmann::json::parse_error if the file content is invalid JSON.
   *
   * @code
   * Json conf = load_file("config.json");
   * std::cout << conf.dump(2);
   * @endcode
   */
  inline Json load_file(const fs::path &path)
  {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
      throw std::runtime_error("Cannot open JSON file: " + path.string());

    // Read entire file into buffer
    std::string buf;
    ifs.seekg(0, std::ios::end);
    const std::streampos sz = ifs.tellg();
    if (sz > 0)
    {
      buf.resize(static_cast<std::size_t>(sz));
      ifs.seekg(0, std::ios::beg);
      ifs.read(buf.data(), sz);
    }

    return Json::parse(buf);
  }

  /**
   * @brief Safe version of `load_file()`.
   *
   * Returns `std::nullopt` on error (I/O or invalid JSON).
   *
   * @param path Path to the file to parse.
   * @return `std::optional<Json>` — empty on failure.
   *
   * @code
   * if (auto cfg = try_load_file("config.json")) {
   *     std::cout << (*cfg)["app"] << std::endl;
   * } else {
   *     std::cerr << "Failed to load config" << std::endl;
   * }
   * @endcode
   */
  inline std::optional<Json> try_load_file(const fs::path &path) noexcept
  {
    try
    {
      return load_file(path);
    }
    catch (...)
    {
      return std::nullopt;
    }
  }

  /// @overload for C-string input.
  inline Json loads(const char *s) { return loads(std::string_view{s}); }

  /// @overload for C-string input, safe version.
  inline std::optional<Json> try_loads(const char *s) noexcept
  {
    return try_loads(std::string_view{s});
  }

  /// @overload for C-string file path.
  inline Json load_file(const char *path) { return load_file(fs::path{path}); }

  /// @overload for C-string file path, safe version.
  inline std::optional<Json> try_load_file(const char *path) noexcept
  {
    return try_load_file(fs::path{path});
  }

} // namespace vix::json

#endif // VIX_LOADS_HPP
