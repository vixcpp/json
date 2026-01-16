/**
 *
 *  @file dumps.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_DUMPS_HPP
#define VIX_DUMPS_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <system_error>

/**
 * @file VIX_DUMPS_HPP
 * @brief Utilities for serializing and writing JSON data in Vix.cpp.
 *
 * This header provides convenient helpers to:
 * - Convert JSON values into formatted or compact strings (`dumps`, `dumps_compact`).
 * - Write JSON files atomically to disk (`dump_file`).
 *
 * The helpers wrap `nlohmann::json::dump()` with simplified parameters
 * and include robust file-writing logic to prevent corruption in case of interruption.
 *
 * ### Example
 * @code
 * using namespace vix::json;
 *
 * Json j = {
 *     {"name", "Softadastra"},
 *     {"version", "1.0.0"},
 *     {"features", {"decentralized", "fast", "secure"}}
 * };
 *
 * std::string pretty = dumps_pretty(j);
 * std::string compact = dumps_compact(j);
 *
 * dump_file("data/config.json", j);
 * @endcode
 */

namespace vix::json
{
  /// Alias utilitaire vers `nlohmann::json`.
  using Json = nlohmann::json;

  /// Alias du namespace `std::filesystem` pour concision.
  namespace fs = std::filesystem;

  /**
   * @brief Convert a JSON value into a human-readable string (pretty-printed).
   *
   * @param j JSON object or array.
   * @param indent Number of spaces per indentation level (default: 2).
   * @param ensure_ascii If true, non-ASCII characters are escaped.
   * @return Formatted JSON string.
   *
   * @code
   * Json j = {{"id", 1}, {"name", "Alice"}};
   * std::string s = dumps(j); // → "{\n  \"id\": 1,\n  \"name\": \"Alice\"\n}"
   * @endcode
   */
  inline std::string dumps(const Json &j, int indent = 2, bool ensure_ascii = false)
  {
    return j.dump(indent, ' ', ensure_ascii);
  }

  /**
   * @brief Convert a JSON value into a compact single-line string.
   *
   * @param j JSON object or array.
   * @param ensure_ascii If true, non-ASCII characters are escaped.
   * @return Compact JSON string.
   *
   * @code
   * Json j = {{"x", 1}, {"y", 2}};
   * std::string s = dumps_compact(j); // → "{\"x\":1,\"y\":2}"
   * @endcode
   */
  inline std::string dumps_compact(const Json &j, bool ensure_ascii = false)
  {
    return j.dump(-1, ' ', ensure_ascii);
  }

  /**
   * @brief Alias of `dumps()` for explicit readability.
   *
   * @param j JSON object or array.
   * @param indent Number of spaces per indentation level.
   * @param ensure_ascii Escape non-ASCII characters if true.
   * @return Pretty-printed JSON string.
   */
  inline std::string dumps_pretty(const Json &j, int indent = 2, bool ensure_ascii = false)
  {
    return dumps(j, indent, ensure_ascii);
  }

  /**
   * @brief Write a JSON value to a file atomically.
   *
   * The function writes to a temporary file (`.tmp` suffix), then renames it
   * to the final destination to avoid corruption in case of interruption.
   * It also ensures that the parent directory exists.
   *
   * @param path Destination file path.
   * @param j JSON value to write.
   * @param indent Number of spaces per indentation level (default: 2).
   * @param ensure_ascii If true, escape non-ASCII characters.
   *
   * @throws std::runtime_error if the file cannot be written or renamed.
   *
   * @code
   * Json conf = {{"app", "Vix.cpp"}, {"debug", true}};
   * dump_file("output/settings.json", conf);
   * @endcode
   */
  inline void dump_file(const fs::path &path, const Json &j, int indent = 2, bool ensure_ascii = false)
  {
    // 1) Ensure directory exists
    if (path.has_parent_path())
    {
      std::error_code ec;
      fs::create_directories(path.parent_path(), ec); // best-effort
    }

    // 2) Write to temp file
    const fs::path tmp = path.string() + ".tmp";
    {
      std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
      if (!ofs)
        throw std::runtime_error("Cannot open temp file for writing: " + tmp.string());

      ofs.exceptions(std::ofstream::badbit | std::ofstream::failbit);

      try
      {
        const auto s = j.dump(indent, ' ', ensure_ascii);
        ofs.write(s.data(), static_cast<std::streamsize>(s.size()));
        ofs.flush();
        if (!ofs)
          throw std::runtime_error("Failed to flush JSON to temp file: " + tmp.string());
      }
      catch (...)
      {
        std::error_code ec;
        fs::remove(tmp, ec);
        throw;
      }
    }

    // 3) Replace destination atomically
    std::error_code ec;
    if (fs::exists(path, ec))
    {
      fs::remove(path, ec); // best-effort
    }
    fs::rename(tmp, path, ec);
    if (ec)
    {
      // fallback: copy + remove
      std::error_code ec2;
      fs::copy_file(tmp, path, fs::copy_options::overwrite_existing, ec2);
      fs::remove(tmp, ec2);
      if (ec2)
        throw std::runtime_error("Failed to move JSON temp file to destination: " + path.string());
    }
  }

  /**
   * @brief Overload accepting a C-string path.
   */
  inline void dump_file(const char *path, const Json &j, int indent = 2, bool ensure_ascii = false)
  {
    dump_file(fs::path{path}, j, indent, ensure_ascii);
  }

  /**
   * @brief Overload accepting a `std::string` path.
   */
  inline void dump_file(const std::string &path, const Json &j, int indent = 2, bool ensure_ascii = false)
  {
    dump_file(fs::path{path}, j, indent, ensure_ascii);
  }

} // namespace vix::json

#endif // VIX_DUMPS_HPP
