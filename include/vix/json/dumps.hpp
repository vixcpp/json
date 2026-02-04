/**
 *
 *  @file dumps.hpp
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
#ifndef VIX_JSON_DUMPS_HPP
#define VIX_JSON_DUMPS_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <stdexcept>

/**
 * @file dumps.hpp
 * @brief JSON serialization helpers (to string & to file) for Vix.cpp.
 *
 * @details
 * This header provides two categories of helpers:
 *
 * 1) **Serialize JSON to a string**
 * - `dumps()` produces pretty JSON (multi-line).
 * - `dumps_compact()` produces compact JSON (single-line).
 * - `dumps_pretty()` is an explicit alias of `dumps()`.
 *
 * 2) **Write JSON to disk safely**
 * - `dump_file()` writes a JSON document using a temp file + rename strategy.
 *
 * ---
 *
 * ## For beginners
 * If you want to write a JSON file safely:
 * @code
 * using namespace vix::json;
 * Json j = {{"app", "Vix.cpp"}, {"debug", true}};
 * dump_file("config.json", j);
 * @endcode
 *
 * ---
 *
 * ## For advanced users
 * - `dump_file()` writes to `<path>.tmp` then renames to `<path>`.
 * - This reduces the risk of corrupted files if the process crashes mid-write.
 * - The function also creates parent directories (best-effort).
 *
 * Note: "atomic" depends on filesystem semantics. On typical local filesystems,
 * rename within the same directory is atomic, but the function still provides a
 * safe fallback when rename fails.
 */

namespace vix::json
{
  /// Primary JSON type used across Vix.cpp.
  using Json = nlohmann::json;

  /// Filesystem alias for convenience.
  namespace fs = std::filesystem;

  /**
   * @brief Serialize JSON to a human-readable string (pretty printed).
   *
   * @param j JSON value (object/array/etc).
   * @param indent Number of spaces per indentation level (default: 2).
   * @param ensure_ascii If true, non-ASCII characters are escaped.
   * @return Formatted JSON string.
   *
   * @note
   * Use this for logs, debugging, configuration files, and readable outputs.
   */
  inline std::string dumps(const Json &j, int indent = 2, bool ensure_ascii = false)
  {
    return j.dump(indent, ' ', ensure_ascii);
  }

  /**
   * @brief Serialize JSON to a compact single-line string.
   *
   * @param j JSON value (object/array/etc).
   * @param ensure_ascii If true, non-ASCII characters are escaped.
   * @return Compact JSON string.
   *
   * @note
   * Use this for network payloads, compact storage, or performance-sensitive logs.
   */
  inline std::string dumps_compact(const Json &j, bool ensure_ascii = false)
  {
    return j.dump(-1, ' ', ensure_ascii);
  }

  /**
   * @brief Explicit alias for dumps() to emphasize readability.
   */
  inline std::string dumps_pretty(const Json &j, int indent = 2, bool ensure_ascii = false)
  {
    return dumps(j, indent, ensure_ascii);
  }

  /**
   * @brief Write JSON to a file using a temp file + rename strategy.
   *
   * @details
   * Steps:
   * 1) Ensure parent directory exists (best-effort).
   * 2) Write JSON to `<path>.tmp`.
   * 3) Replace destination by renaming temp file to `<path>`.
   * 4) If rename fails, fallback to copy + remove.
   *
   * This strategy reduces the chance of ending with a partially-written file.
   *
   * @param path Destination file path.
   * @param j JSON value to write.
   * @param indent Pretty-print indentation (default: 2).
   * @param ensure_ascii Escape non-ASCII characters if true.
   *
   * @throws std::runtime_error
   *         If the temp file cannot be written or the final replacement fails.
   *
   * @warning
   * This function writes the entire JSON document at once.
   * Do not use it for unbounded user uploads.
   */
  inline void dump_file(const fs::path &path, const Json &j, int indent = 2, bool ensure_ascii = false)
  {
    // 1) Ensure directory exists (best-effort)
    if (path.has_parent_path())
    {
      std::error_code ec;
      fs::create_directories(path.parent_path(), ec);
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

    // 3) Replace destination
    std::error_code ec;
    if (fs::exists(path, ec))
      fs::remove(path, ec); // best-effort

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

  /// @overload Convenience overload for C-string path.
  inline void dump_file(const char *path, const Json &j, int indent = 2, bool ensure_ascii = false)
  {
    dump_file(fs::path{path}, j, indent, ensure_ascii);
  }

  /// @overload Convenience overload for std::string path.
  inline void dump_file(const std::string &path, const Json &j, int indent = 2, bool ensure_ascii = false)
  {
    dump_file(fs::path{path}, j, indent, ensure_ascii);
  }

} // namespace vix::json

#endif // VIX_JSON_DUMPS_HPP
