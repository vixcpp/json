#ifndef VIX_BUILD_HPP
#define VIX_BUILD_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

/**
 * @file VIX_BUILD_HPP
 * @brief JSON construction helpers for Vix.cpp based on *nlohmann/json*.
 *
 * Provides a minimal, expressive syntax to build JSON objects and arrays inline
 * without verbose boilerplate. Intended for use in API responses, tests, and
 * configuration generation.
 *
 * ### Features
 * - `o(k1, v1, k2, v2, …)` → builds a JSON object.
 * - `a(v1, v2, v3, …)` → builds a JSON array.
 * - `kv({{"key", value}, ...})` → builds an object from a key/value initializer list.
 *
 * ### Example
 * @code
 * using namespace vix::json;
 *
 * Json user = o(
 *     "id", 42,
 *     "name", "Gaspard",
 *     "skills", a("C++", "Blockchain", "AI")
 * );
 *
 * std::cout << user.dump(2);
 * // {
 * //   "id": 42,
 * //   "name": "Gaspard",
 * //   "skills": ["C++", "Blockchain", "AI"]
 * // }
 *
 * Json meta = kv({
 *     {"version", "1.0.0"},
 *     {"active", true}
 * });
 * @endcode
 *
 * @note This header is header-only and has no dependencies other than *nlohmann/json*.
 */

namespace vix::json
{
    /**
     * @brief Alias to simplify use of nlohmann::json.
     */
    using Json = nlohmann::json;
    using OrderedJson = nlohmann::ordered_json;

    // ---------------------------------------------------------------------
    // Internal utilities (not for direct use)
    // ---------------------------------------------------------------------
    namespace detail
    {
        /**
         * @brief Compile-time trait for key-like types (convertible to `std::string_view`).
         */
        template <class K>
        using is_key_like = std::bool_constant<std::is_convertible_v<K, std::string_view>>;

        inline void put_pairs(Json &) {}

        /**
         * @brief Internal recursive helper that inserts key/value pairs into a JSON object.
         *
         * @tparam K Key type (must be convertible to `std::string_view`)
         * @tparam V Value type (any JSON-compatible type)
         * @tparam Rest Remaining key/value pairs
         *
         * @param j JSON object being filled
         * @param k Current key
         * @param v Current value
         * @param rest Remaining pairs
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

    // ---------------------------------------------------------------------
    // JSON object builder: o(k1, v1, k2, v2, ...)
    // ---------------------------------------------------------------------

    /**
     * @brief Build a JSON object from a variadic list of key/value pairs.
     *
     * Each key must be convertible to `std::string_view`.
     * The number of arguments must be even: `(k1, v1, k2, v2, …)`.
     *
     * @tparam Args Key/value argument pack.
     * @param args Sequence of alternating keys and values.
     * @return Constructed JSON object.
     *
     * @code
     * Json obj = o("name", "Alice", "age", 30);
     * // → {"name": "Alice", "age": 30}
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

    // ---------------------------------------------------------------------
    // JSON array builder: a(v1, v2, v3, ...)
    // ---------------------------------------------------------------------

    /**
     * @brief Build a JSON array from a variadic list of values.
     *
     * @tparam Ts Value types (each must be JSON-serializable).
     * @param ts Sequence of elements.
     * @return Constructed JSON array.
     *
     * @code
     * Json arr = a(1, 2, 3, 4);
     * // → [1, 2, 3, 4]
     * @endcode
     */
    template <class... Ts>
    inline Json a(Ts &&...ts)
    {
        Json arr = Json::array();
        (arr.push_back(Json(std::forward<Ts>(ts))), ...);
        return arr;
    }

    // ---------------------------------------------------------------------
    // Key/value initializer-list builder: kv({{"key", value}, ...})
    // ---------------------------------------------------------------------

    /**
     * @brief Build a JSON object from an initializer list of key/value pairs.
     *
     * This function provides an explicit syntax for constructing
     * objects when the number of pairs is not known at compile time.
     *
     * @param xs List of key/value pairs (`std::string_view`, `Json`).
     * @return Constructed JSON object.
     *
     * @code
     * Json j = kv({
     *   {"version", "1.0.0"},
     *   {"debug", true}
     * });
     * // → {"version": "1.0.0", "debug": true}
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

#endif // VIX_BUILD_HPP
