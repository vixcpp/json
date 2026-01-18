/**
 *
 *  @file convert.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_CONVERT_HPP
#define VIX_CONVERT_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <stdexcept>

/**
 * @file VIX_CONVERT_HPP
 * @brief Safe JSON accessors and converters built on top of *nlohmann/json*.
 *
 * Utilities to navigate and extract values from JSON documents with:
 * - **Pointer helpers** `ptr()` that return a `const Json*` (or `nullptr`)
 * - **Optional getters** `get_opt<T>()` that never throw
 * - **Defaulted getters** `get_or<T>()` returning a fallback value
 * - **Strict getters** `ensure<T>()` that throw with clear messages
 *
 * These helpers avoid repetitive `contains()/is_*()` checks and centralize
 * error handling. They are especially useful for parsing external payloads.
 *
 * ### Example
 * @code
 * using namespace vix::json;
 *
 * Json j = R"({
 *   "user": {"id": 42, "name": "Ada"},
 *   "tags": ["c++", "ai"]
 * })"_json;
 *
 * // Pointers
 * const Json* p_user = ptr(j, "user");                // -> non-null
 * const Json* p_tag0 = ptr(j["tags"], 0);             // -> non-null
 *
 * // Optionals
 * auto id   = get_opt<int>(*p_user, "id");            // -> std::optional<int>{42}
 * auto city = get_opt<std::string>(*p_user, "city");  // -> std::nullopt
 *
 * // Defaults
 * int safe_id = get_or<int>(*p_user, "id", -1);       // -> 42
 * std::string name = get_or<std::string>(*p_user, "name", "unknown"); // -> "Ada"
 *
 * // Strict (throws on missing/type mismatch)
 * int must_id = ensure<int>(*p_user, "id");
 * @endcode
 */

namespace vix::json
{
  /// Alias utilitaire vers `nlohmann::json`.
  using Json = nlohmann::json;

  /**
   * @brief Obtain a pointer to a member value by key if `j` is an object.
   *
   * @param j   JSON value expected to be an object.
   * @param key Object key to look up.
   * @return Pointer to the member value, or `nullptr` if `j` is not an object
   *         or the key does not exist.
   */
  inline const Json *ptr(const Json &j, std::string_view key) noexcept
  {
    if (!j.is_object())
      return nullptr;
    auto it = j.find(std::string(key));
    return (it == j.end()) ? nullptr : &(*it);
  }

  /**
   * @brief Obtain a pointer to an array element by index if `j` is an array.
   *
   * @param j   JSON value expected to be an array.
   * @param idx Zero-based index.
   * @return Pointer to the element, or `nullptr` if `j` is not an array or
   *         `idx` is out of bounds.
   */
  inline const Json *ptr(const Json &j, std::size_t idx) noexcept
  {
    if (!j.is_array())
      return nullptr;
    return (idx < j.size()) ? &j[idx] : nullptr;
  }

  /**
   * @brief Try to convert `j` to `T`, returning `std::nullopt` on failure.
   *
   * Returns `std::nullopt` if `j` is discarded or `null`, or if conversion
   * throws a `nlohmann::json::exception`.
   *
   * @tparam T Target type (must be JSON-convertible).
   * @param j  JSON value.
   * @return `std::optional<T>`.
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
   * @brief Try to convert `*jp` to `T`, or `std::nullopt` if `jp` is null.
   */
  template <class T>
  inline std::optional<T> get_opt(const Json *jp) noexcept
  {
    return (jp ? get_opt<T>(*jp) : std::optional<T>{});
  }

  /**
   * @brief Try to convert `obj[key]` to `T`, or `std::nullopt` if missing/invalid.
   */
  template <class T>
  inline std::optional<T> get_opt(const Json &obj, std::string_view key) noexcept
  {
    return get_opt<T>(ptr(obj, key));
  }

  /**
   * @brief Try to convert `arr[idx]` to `T`, or `std::nullopt` if missing/invalid.
   */
  template <class T>
  inline std::optional<T> get_opt(const Json &arr, std::size_t idx) noexcept
  {
    return get_opt<T>(ptr(arr, idx));
  }

  /**
   * @brief Convert `j` to `T` or return `def` if not possible.
   *
   * @tparam T  Target type.
   * @param j   JSON value.
   * @param def Default value to return on failure.
   * @return `T` either parsed or defaulted.
   */
  template <class T>
  inline T get_or(const Json &j, T def) noexcept
  {
    auto v = get_opt<T>(j);
    return v ? std::move(*v) : std::move(def);
  }

  /**
   * @brief Convert `*jp` to `T` or return `def` if `jp` is null or parsing fails.
   */
  template <class T>
  inline T get_or(const Json *jp, T def) noexcept
  {
    return jp ? get_or<T>(*jp, std::move(def)) : std::move(def);
  }

  /**
   * @brief Convert `obj[key]` to `T` or return `def` if missing/invalid.
   */
  template <class T>
  inline T get_or(const Json &obj, std::string_view key, T def) noexcept
  {
    auto v = get_opt<T>(obj, key);
    return v ? std::move(*v) : std::move(def);
  }

  /**
   * @brief Convert `arr[idx]` to `T` or return `def` if missing/invalid.
   */
  template <class T>
  inline T get_or(const Json &arr, std::size_t idx, T def) noexcept
  {
    auto v = get_opt<T>(arr, idx);
    return v ? std::move(*v) : std::move(def);
  }

  /**
   * @brief Strictly convert `j` to `T`, letting *nlohmann/json* exceptions propagate.
   *
   * Use this when the value must exist and be of the right type.
   *
   * @tparam T Target type.
   * @param j  JSON value.
   * @return Parsed value of type `T`.
   * @throws nlohmann::json::exception on conversion error.
   */
  template <class T>
  inline T ensure(const Json &j)
  {
    // Let nlohmann::json exceptions propagate (type mismatch, etc.)
    return j.get<T>();
  }

  /**
   * @brief Strictly convert `obj[key]` to `T`, throwing clear `std::runtime_error`s.
   *
   * Checks that `obj` is an object and `key` exists; wraps the underlying
   * `nlohmann::json` exception with a contextual message indicating which key
   * failed conversion.
   *
   * @tparam T   Target type.
   * @param obj  JSON value expected to be an object.
   * @param key  Object key to extract.
   * @return Parsed value of type `T`.
   *
   * @throws std::runtime_error if `obj` is not an object or `key` is missing.
   * @throws std::runtime_error wrapping `nlohmann::json::exception` on type error.
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

#endif // VIX_CONVERT_HPP
