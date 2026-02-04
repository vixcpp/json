/**
 *
 *  @file build.hpp
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

// Backward-compatible guard alias.
// Some users might (rarely) rely on VIX_BUILD_HPP in preprocessor checks.
#ifndef VIX_BUILD_HPP
#define VIX_BUILD_HPP
#endif

#ifndef VIX_JSON_BUILD_HPP
#define VIX_JSON_BUILD_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

/**
 * @file build.hpp
 * @brief JSON construction helpers for Vix.cpp (built on nlohmann::json).
 *
 * @details
 * This header provides small, convenient builders to construct JSON values with
 * less boilerplate.
 *
 * It is designed for:
 * - API responses
 * - tests and fixtures
 * - configuration generation
 *
 * The goal is to keep code readable for beginners while remaining practical
 * for advanced users.
 *
 * ---
 *
 * ## What you get
 *
 * - `o(k1, v1, k2, v2, ...)` builds an object (ordered).
 * - `a(v1, v2, v3, ...)` builds an array.
 * - `kv({{"k", Json(...)}, ...})` builds an object from a list of pairs.
 *
 * ---
 *
 * ## Beginner example
 * @code
 * #include <vix/json/build.hpp>
 * using namespace vix::json;
 *
 * auto user = o(
 *   "id", 42,
 *   "name", "Gaspard",
 *   "skills", a("C++", "Networking", "Systems")
 * );
 *
 * std::cout << user.dump(2) << "\n";
 * @endcode
 *
 * ---
 *
 * ## Important notes
 * - `o()` returns `OrderedJson` to keep deterministic key order in output.
 * - All builders are header-only and only depend on nlohmann::json.
 * - Keys passed to `o()` must be convertible to `std::string_view`.
 *
 * ---
 *
 * ## Common mistakes
 * - `o("a", 1, "b")` is invalid (odd number of arguments). It triggers a compile error.
 * - `kv({{"a", 1}})` is invalid because the value type must be `Json`.
 *   Use: `kv({{"a", Json(1)}})` or prefer `o("a", 1)`.
 */

namespace vix::json
{
  /// Primary JSON type used across Vix.cpp.
  using Json = nlohmann::json;

  /// Ordered JSON type for deterministic object key order.
  using OrderedJson = nlohmann::ordered_json;

  namespace detail
  {
    /// Trait: true if K can be viewed as a string key.
    template <class K>
    using is_key_like = std::bool_constant<std::is_convertible_v<K, std::string_view>>;

    // Kept for potential internal overload extensions.
    inline void put_pairs(Json &) {}

    /**
     * @brief Internal helper to insert variadic key/value pairs into an object.
     *
     * @tparam K Key type (must be convertible to std::string_view).
     * @tparam V Value type (must be JSON-serializable).
     * @tparam Rest Remaining arguments.
     */
    template <class K, class V, class... Rest,
              std::enable_if_t<is_key_like<K>::value, int> = 0>
    inline void put_pairs(OrderedJson &j, K &&k, V &&v, Rest &&...rest)
    {
      const std::string key{std::string_view(std::forward<K>(k))};
      j.emplace(key, OrderedJson(std::forward<V>(v)));
      if constexpr (sizeof...(rest) > 0)
        put_pairs(j, std::forward<Rest>(rest)...);
    }
  } // namespace detail

  /**
   * @brief Build an ordered JSON object from (key, value) pairs.
   *
   * Keys must be convertible to std::string_view.
   * The number of arguments must be even.
   *
   * @tparam Args Arguments pack: k1, v1, k2, v2, ...
   * @param args Alternating keys and values.
   * @return OrderedJson object.
   *
   * @code
   * auto j = o("name", "Alice", "age", 30);
   * @endcode
   */
  template <class... Args>
  inline OrderedJson o(Args &&...args)
  {
    static_assert(sizeof...(args) % 2 == 0,
                  "json::o requires an even number of args: (k1,v1,k2,v2,...)");
    OrderedJson j = OrderedJson::object();
    if constexpr (sizeof...(args) > 0)
      detail::put_pairs(j, std::forward<Args>(args)...);
    return j;
  }

  /**
   * @brief Build a JSON array from values.
   *
   * @tparam Ts Value types (must be JSON-serializable).
   * @param ts Elements.
   * @return Json array.
   *
   * @code
   * auto xs = a(1, 2, 3);
   * @endcode
   */
  template <class... Ts>
  inline Json a(Ts &&...ts)
  {
    Json arr = Json::array();
    (arr.push_back(Json(std::forward<Ts>(ts))), ...);
    return arr;
  }

  /**
   * @brief Build a JSON object from a list of (key, Json) pairs.
   *
   * This is useful when pairs are not known at compile time.
   * If you can, prefer `o()` for simplicity.
   *
   * @param xs List of pairs: {{"k", Json(...)}, ...}
   * @return Json object.
   *
   * @code
   * Json j = kv({
   *   {"version", Json("1.0.0")},
   *   {"debug", Json(true)}
   * });
   * @endcode
   */
  inline Json kv(std::initializer_list<std::pair<std::string_view, Json>> xs)
  {
    Json j = Json::object();
    for (const auto &p : xs)
      j.emplace(std::string(p.first), p.second);
    return j;
  }

} // namespace vix::json

#endif // VIX_JSON_BUILD_HPP
