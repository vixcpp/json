/**
 *
 *  @file Simple.hpp
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
 *
 */
#ifndef VIX_JSON_SIMPLE_HPP
#define VIX_JSON_SIMPLE_HPP

/**
 * @file Simple.hpp
 * @brief Minimal JSON-like data model for lightweight Vix internal APIs.
 *
 * @details
 * vix::json::Simple provides a self-contained JSON representation for internal
 * use, independent from nlohmann::json. It is designed to be header-only and
 * easily embeddable in performance-sensitive modules or plugins.
 *
 * Features:
 * - token: a tagged variant supporting scalars, arrays, and objects.
 * - array_t: a flat sequence of tokens representing JSON arrays.
 * - kvs: a flattened key/value list representing JSON objects.
 * - Helper functions (obj() and array()) for quick construction.
 * - Implicit constructors for nesting (token can wrap array_t or kvs).
 *
 * Example:
 * @code
 * #include <vix/json/Simple.hpp>
 *
 * using namespace vix::json;
 *
 * kvs user = obj({
 *   "name", "Alice",
 *   "age", 30,
 *   "skills", array({"C++", "Networking", "Systems"})
 * });
 *
 * token t = user; // convertible to token
 * @endcode
 *
 * Design notes:
 * - This type avoids JSON parsing and serialization overhead when moving data
 *   between internal adapters.
 * - Recursive types are represented using shared_ptr wrappers to keep token
 *   trivially copyable.
 */

#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <initializer_list>
#include <type_traits>

namespace vix::json
{
  /**
   * @brief Forward declaration for JSON array representation.
   */
  struct array_t;

  /**
   * @brief Forward declaration for JSON object representation (flattened kv pairs).
   */
  struct kvs;

  /**
   * @struct token
   * @brief A tagged variant representing a JSON-like value.
   *
   * Supported types:
   * - null (std::monostate)
   * - bool
   * - integer (long long)
   * - floating point (double)
   * - string (std::string)
   * - array (std::shared_ptr<array_t>)
   * - object (std::shared_ptr<kvs>)
   *
   * Arrays and objects are stored via shared_ptr to allow recursion while
   * keeping token copyable and lightweight.
   */
  struct token
  {
    /**
     * @brief Underlying variant type for the token.
     */
    using value_t = std::variant<
        std::monostate,
        bool,
        long long,
        double,
        std::string,
        std::shared_ptr<array_t>,
        std::shared_ptr<kvs>>;

    /**
     * @brief Stored value.
     */
    value_t v{std::monostate{}};

    /// @brief Default constructs a null token.
    token() = default;

    /// @brief Construct a null token.
    token(std::nullptr_t) : v(std::monostate{}) {}

    /// @brief Construct a boolean token.
    token(bool b) : v(b) {}

    /// @brief Construct an integer token (int promoted to 64-bit).
    token(int i) : v(static_cast<long long>(i)) {}

    /// @brief Construct an integer token (64-bit).
    token(long long i) : v(i) {}

    /// @brief Construct a floating point token.
    token(double d) : v(d) {}

    /// @brief Construct a string token from a C-string.
    token(const char *s) : v(std::string(s)) {}

    /// @brief Construct a string token by value (moved into storage).
    token(std::string s) : v(std::move(s)) {}

    /**
     * @brief Construct an object token from kvs.
     *
     * @param obj Object representation.
     */
    token(const kvs &obj);

    /**
     * @brief Construct an array token from array_t.
     *
     * @param arr Array representation.
     */
    token(const array_t &arr);
  };

  /**
   * @struct kvs
   * @brief Flattened key/value list representing a JSON object.
   *
   * The vector is expected to contain alternating tokens:
   * key0, value0, key1, value1, ...
   * Keys are typically string tokens.
   */
  struct kvs
  {
    /**
     * @brief Flat key/value token list.
     */
    std::vector<token> flat{};

    /// @brief Default construct an empty object.
    kvs() = default;

    /**
     * @brief Construct from an initializer list.
     *
     * @param list Tokens in flattened key/value order.
     */
    kvs(std::initializer_list<token> list) : flat(list) {}

    /**
     * @brief Construct from a vector (copy).
     *
     * @param v Token vector.
     */
    explicit kvs(const std::vector<token> &v) : flat(v) {}

    /**
     * @brief Construct from a vector (move).
     *
     * @param v Token vector.
     */
    explicit kvs(std::vector<token> &&v) : flat(std::move(v)) {}
  };

  /**
   * @struct array_t
   * @brief Token list representing a JSON array.
   */
  struct array_t
  {
    /**
     * @brief Array elements.
     */
    std::vector<token> elems;

    /// @brief Default construct an empty array.
    array_t() = default;

    /**
     * @brief Construct from an initializer list.
     *
     * @param l List of elements.
     */
    array_t(std::initializer_list<token> l) : elems(l) {}

    /**
     * @brief Construct from a vector (copy).
     *
     * @param v Element vector.
     */
    explicit array_t(const std::vector<token> &v) : elems(v) {}

    /**
     * @brief Construct from a vector (move).
     *
     * @param v Element vector.
     */
    explicit array_t(std::vector<token> &&v) : elems(std::move(v)) {}
  };

  /**
   * @brief Create an object token by storing a shared_ptr to a kvs copy.
   */
  inline token::token(const kvs &obj) : v(std::make_shared<kvs>(obj)) {}

  /**
   * @brief Create an array token by storing a shared_ptr to an array_t copy.
   */
  inline token::token(const array_t &arr) : v(std::make_shared<array_t>(arr)) {}

  /**
   * @brief Helper to create an array_t from an initializer list.
   *
   * @param l Elements.
   * @return array_t
   */
  inline array_t array(std::initializer_list<token> l) { return array_t{l}; }

  /**
   * @brief Helper to create a kvs from an initializer list.
   *
   * Tokens must be passed in flattened key/value order.
   *
   * @param l Tokens.
   * @return kvs
   */
  inline kvs obj(std::initializer_list<token> l) { return kvs{l}; }

  /**
   * @brief Helper to create an array_t from a vector (copy).
   *
   * @param v Elements.
   * @return array_t
   */
  inline array_t array(const std::vector<token> &v) { return array_t{v}; }

  /**
   * @brief Helper to create an array_t from a vector (move).
   *
   * @param v Elements.
   * @return array_t
   */
  inline array_t array(std::vector<token> &&v) { return array_t{std::move(v)}; }

  /**
   * @brief Helper to create a kvs from a vector (copy).
   *
   * @param v Tokens.
   * @return kvs
   */
  inline kvs obj(const std::vector<token> &v) { return kvs{v}; }

  /**
   * @brief Helper to create a kvs from a vector (move).
   *
   * @param v Tokens.
   * @return kvs
   */
  inline kvs obj(std::vector<token> &&v) { return kvs{std::move(v)}; }

} // namespace vix::json

#endif // VIX_JSON_SIMPLE_HPP
