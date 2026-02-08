/**
 *
 *  @file loads.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/vixcpp/vix
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_JSON_LOADS_HPP
#define VIX_JSON_LOADS_HPP

#include <nlohmann/json.hpp>
#include <string_view>
#include <optional>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <string>

/**
 * @file loads.hpp
 * @brief JSON parsing helpers (string & file) for Vix.cpp.
 *
 * @details
 * This header provides **simple, safe and explicit helpers** to parse JSON
 * from strings or files.
 *
 * It is designed for:
 * - **Beginners** who want a clear API without dealing with exceptions everywhere.
 * - **Advanced users** who need full control over error handling and performance.
 *
 * The API is intentionally small and explicit:
 * you always choose between a **throwing** version or a **safe optional** version.
 *
 * ---
 *
 * ## Available functions
 *
 * ### From strings
 * - `loads()`
 *   Parses JSON and **throws on error**.
 *
 * - `try_loads()`
 *   Parses JSON and returns `std::nullopt` on error (never throws).
 *
 * ### From files
 * - `load_file()`
 *   Loads and parses a JSON file, **throws on error**.
 *
 * - `try_load_file()`
 *   Safe version returning `std::nullopt` on failure.
 *
 * ---
 *
 * ## When to use what?
 *
 * | Situation | Recommended function |
 * |----------|---------------------|
 * | Config file must exist | `load_file()` |
 * | User input / external data | `try_loads()` |
 * | Tests or trusted data | `loads()` |
 * | Optional config file | `try_load_file()` |
 *
 * ---
 *
 * ## Error model
 *
 * - Throwing functions propagate:
 *   - `std::runtime_error` (I/O errors)
 *   - `nlohmann::json::parse_error` (invalid JSON)
 *
 * - `try_*` functions:
 *   - Never throw
 *   - Return `std::nullopt` on **any** failure
 *
 * ---
 *
 * ## Example
 * @code
 * using namespace vix::json;
 *
 * // Parse JSON from string
 * Json j = loads(R"({"id": 1, "name": "Softadastra"})");
 *
 * // Safe parsing
 * if (auto maybe = try_loads("not-json")) {
 *   std::cout << maybe->dump(2);
 * } else {
 *   std::cout << "Invalid JSON";
 * }
 *
 * // Parse JSON file
 * Json cfg = load_file("config.json");
 *
 * // Optional file
 * if (auto data = try_load_file("settings.json")) {
 *   std::cout << "Loaded " << data->size() << " entries";
 * }
 * @endcode
 */

namespace vix::json
{
  /// Primary JSON type used across Vix.cpp.
  using Json = nlohmann::json;

  /// Filesystem alias for convenience.
  namespace fs = std::filesystem;

  /**
   * @brief Parse a JSON document from a string view.
   *
   * @param s UTF-8 encoded JSON text.
   * @return Parsed JSON value.
   *
   * @throws nlohmann::json::parse_error
   *         If the input is not valid JSON.
   *
   * @note
   * Use this function when invalid JSON is a **programming error**
   * and should abort execution.
   */
  inline Json loads(std::string_view s)
  {
    return Json::parse(s);
  }

  /**
   * @brief Safe JSON parsing from a string.
   *
   * @param s UTF-8 encoded JSON text.
   * @return Parsed JSON or `std::nullopt` on error.
   *
   * @note
   * This function never throws.
   * Recommended for user input or external data.
   */
  inline std::optional<Json> try_loads(std::string_view s) noexcept
  {
    try
    {
      Json j = Json::parse(s);
      return std::optional<Json>{std::move(j)};
    }
    catch (...)
    {
      return std::nullopt;
    }
  }

  /**
   * @brief Load and parse a JSON file.
   *
   * @param path Path to a JSON file.
   * @return Parsed JSON document.
   *
   * @throws std::runtime_error
   *         If the file cannot be opened or is empty.
   * @throws nlohmann::json::parse_error
   *         If the file content is not valid JSON.
   *
   * @note
   * This function reads the entire file into memory.
   * It is intended for configuration and metadata files,
   * not for unbounded user uploads.
   */
  inline Json load_file(const fs::path &path)
  {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
      throw std::runtime_error("Cannot open JSON file: " + path.string());

    ifs.seekg(0, std::ios::end);
    const std::streampos end = ifs.tellg();
    if (end <= 0)
      throw std::runtime_error("Empty JSON file: " + path.string());

    std::string buf;
    buf.resize(static_cast<std::size_t>(end));

    ifs.seekg(0, std::ios::beg);
    ifs.read(buf.data(), static_cast<std::streamsize>(buf.size()));

    return Json::parse(buf);
  }

  /**
   * @brief Safe version of load_file().
   *
   * @param path Path to a JSON file.
   * @return Parsed JSON or `std::nullopt` on failure.
   *
   * @note
   * This function never throws.
   */
  inline std::optional<Json> try_load_file(const fs::path &path) noexcept
  {
    try
    {
      auto j = load_file(path);
      return std::optional<Json>{std::move(j)};
    }
    catch (...)
    {
      return std::nullopt;
    }
  }

  /// @overload Convenience overload for C-string input.
  inline Json loads(const char *s) { return loads(std::string_view{s}); }

  /// @overload Safe C-string version.
  inline std::optional<Json> try_loads(const char *s) noexcept
  {
    return try_loads(std::string_view{s});
  }

  /// @overload Convenience overload for C-string file path.
  inline Json load_file(const char *path) { return load_file(fs::path{path}); }

  /// @overload Safe C-string file version.
  inline std::optional<Json> try_load_file(const char *path) noexcept
  {
    return try_load_file(fs::path{path});
  }

} // namespace vix::json

#endif // VIX_JSON_LOADS_HPP
