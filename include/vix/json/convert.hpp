/**
 *
 *  @file convert.hpp
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
#ifndef VIX_JSON_CONVERT_HPP
#define VIX_JSON_CONVERT_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <stdexcept>

/**
 * @file convert.hpp
 * @brief Safe JSON accessors and converters for Vix.cpp (built on nlohmann::json).
 *
 * @details
 * This header reduces repetitive JSON boilerplate such as:
 * - `contains()`, `is_*()`, try/catch around `get<T>()`
 * - manual checks for missing keys or invalid array indexes
 *
 * It provides four levels of strictness:
 *
 * 1) **ptr()**
 *    Returns a pointer (`const Json*`) or `nullptr` when missing.
 *
 * 2) **get_opt<T>()**
 *    Returns `std::optional<T>`, never throws.
 *
 * 3) **get_or<T>()**
 *    Returns a `T` or a default value when missing/invalid.
 *
 * 4) **ensure<T>()**
 *    Strict: throws when missing/type mismatch.
 *
 * ---
 *
 * ## For beginners
 *
 * Use this simple rule:
 * - External/user input: `get_opt()` or `get_or()`
 * - Internal/trusted data: `ensure()`
 *
 * Example:
 * @code
 * using namespace vix::json;
 *
 * Json j = R"({"user": {"id": 42, "name": "Ada"}})"_json;
 *
 * // safe
 * auto id = get_or<int>(j["user"], "id", -1);
 *
 * // strict
 * int must_id = ensure<int>(j["user"], "id");
 * @endcode
 *
 * ---
 *
 * ## For advanced users
 * - `get_opt()` catches `nlohmann::json::exception` and returns `std::nullopt`.
 * - `ensure()` is intentionally strict; use it when you want failures to be loud.
 * - `ptr(obj, key)` allocates a temporary std::string in this implementation.
 *   This keeps compatibility stable across versions of nlohmann::json.
 */

namespace vix::json
{
  /// Primary JSON type used across Vix.cpp.
  using Json = nlohmann::json;

  /**
   * @brief Pointer to an object member by key.
   *
   * @param j JSON value expected to be an object.
   * @param key Object key to look up.
   * @return Pointer to the member, or nullptr if missing or not an object.
   */
  inline const Json *ptr(const Json &j, std::string_view key) noexcept
  {
    if (!j.is_object())
      return nullptr;

    // Compatible approach across nlohmann::json versions:
    auto it = j.find(std::string(key));
    return (it == j.end()) ? nullptr : &(*it);
  }

  /**
   * @brief Pointer to an array element by index.
   *
   * @param j JSON value expected to be an array.
   * @param idx Zero-based index.
   * @return Pointer to the element, or nullptr if out of bounds or not an array.
   */
  inline const Json *ptr(const Json &j, std::size_t idx) noexcept
  {
    if (!j.is_array())
      return nullptr;
    return (idx < j.size()) ? &j[idx] : nullptr;
  }

  /**
   * @brief Convert a JSON value to T, returning std::nullopt on failure.
   *
   * Failure cases:
   * - `j` is null or discarded
   * - `j.get<T>()` throws (type mismatch, etc.)
   *
   * @tparam T Target type.
   * @param j JSON value.
   * @return std::optional<T>
   */
  template <class T>
  inline std::optional<T> get_opt(const Json &j) noexcept
  {
    try
    {
      if (j.is_discarded() || j.is_null())
        return std::nullopt;
      return j.get<T>();
    }
    catch (const nlohmann::json::exception &)
    {
      return std::nullopt;
    }
  }

  /**
   * @brief Convert *jp to T, or std::nullopt if jp is nullptr or conversion fails.
   */
  template <class T>
  inline std::optional<T> get_opt(const Json *jp) noexcept
  {
    return (jp ? get_opt<T>(*jp) : std::optional<T>{});
  }

  /**
   * @brief Convert obj[key] to T, or std::nullopt if missing/invalid.
   */
  template <class T>
  inline std::optional<T> get_opt(const Json &obj, std::string_view key) noexcept
  {
    return get_opt<T>(ptr(obj, key));
  }

  /**
   * @brief Convert arr[idx] to T, or std::nullopt if missing/invalid.
   */
  template <class T>
  inline std::optional<T> get_opt(const Json &arr, std::size_t idx) noexcept
  {
    return get_opt<T>(ptr(arr, idx));
  }

  /**
   * @brief Convert j to T, or return def if not possible.
   *
   * @tparam T Target type.
   * @param j JSON value.
   * @param def Default value used on failure.
   * @return T parsed or defaulted.
   */
  template <class T>
  inline T get_or(const Json &j, T def) noexcept
  {
    auto v = get_opt<T>(j);
    return v ? std::move(*v) : std::move(def);
  }

  /**
   * @brief Convert *jp to T, or return def if jp is null or conversion fails.
   */
  template <class T>
  inline T get_or(const Json *jp, T def) noexcept
  {
    return jp ? get_or<T>(*jp, std::move(def)) : std::move(def);
  }

  /**
   * @brief Convert obj[key] to T, or return def if missing/invalid.
   */
  template <class T>
  inline T get_or(const Json &obj, std::string_view key, T def) noexcept
  {
    auto v = get_opt<T>(obj, key);
    return v ? std::move(*v) : std::move(def);
  }

  /**
   * @brief Convert arr[idx] to T, or return def if missing/invalid.
   */
  template <class T>
  inline T get_or(const Json &arr, std::size_t idx, T def) noexcept
  {
    auto v = get_opt<T>(arr, idx);
    return v ? std::move(*v) : std::move(def);
  }

  /**
   * @brief Strict conversion of j to T (throws on error).
   *
   * Use when missing/type mismatch is a bug and must be loud.
   *
   * @tparam T Target type.
   * @param j JSON value.
   * @return Parsed T.
   * @throws nlohmann::json::exception on conversion error.
   */
  template <class T>
  inline T ensure(const Json &j)
  {
    return j.get<T>();
  }

  /**
   * @brief Strict conversion of obj[key] to T with clear errors.
   *
   * @throws std::runtime_error if obj is not an object or key is missing.
   * @throws std::runtime_error wrapping nlohmann::json::exception on type mismatch.
   */
  template <class T>
  inline T ensure(const Json &obj, std::string_view key)
  {
    if (!obj.is_object())
      throw std::runtime_error("ensure: not an object");

    auto it = obj.find(std::string(key));
    if (it == obj.end())
      throw std::runtime_error(std::string("ensure: missing key '") + std::string(key) + "'");

    try
    {
      return it->get<T>();
    }
    catch (const nlohmann::json::exception &e)
    {
      throw std::runtime_error(std::string("ensure: type error for key '") + std::string(key) + "': " + e.what());
    }
  }

} // namespace vix::json

#endif // VIX_JSON_CONVERT_HPP
