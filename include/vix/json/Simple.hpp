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

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

/**
 * @file Simple.hpp
 * @brief Minimal JSON-like value model for lightweight internal Vix APIs.
 *
 * @details
 * This header defines a small JSON-like value system used for internal payloads.
 * It is header-only and does not depend on nlohmann::json.
 *
 * It can represent:
 * - null
 * - bool
 * - integer (int64 stored)
 * - floating point (double)
 * - string
 * - arrays (array_t)
 * - objects (kvs)
 *
 * Beginners:
 * - Use this when you need to exchange structured data between modules without parsing text.
 * - If you want to parse or serialize JSON text, use the regular Vix JSON helpers based on nlohmann::json.
 *
 * Experts:
 * - This is a minimal DOM-like model with predictable layout and cheap copying.
 * - Objects are stored as a flat vector: key0, value0, key1, value1, ...
 * - Arrays and objects are stored via shared_ptr inside token, allowing recursion.
 *
 * Compatibility:
 * - The existing public API remains: token, kvs, array_t, obj(), array().
 * - This file only adds helpers and fixes an ambiguity bug. No renames.
 *
 * Important fix:
 * - Some platforms define std::int64_t as long (not long long). In that case,
 *   passing a long long literal or long long variable to token inside an initializer_list
 *   could become ambiguous (convertible to int and to int64_t with same rank).
 * - We add explicit constructors for long long and unsigned long long to remove ambiguity
 *   without changing existing call sites.
 *
 * Quick example:
 * @code
 * using namespace vix::json;
 *
 * kvs user = obj({
 *   "name", "Alice",
 *   "age", 30,
 *   "skills", array({"C++", "Networking"})
 * });
 *
 * token root = user;
 * root.ensure_object().set_string("country", "UG");
 *
 * root.ensure_object().for_each_pair([](std::string_view k, const token& v){
 *   (void)k; (void)v;
 * });
 * @endcode
 */

namespace vix::json
{
  struct array_t;
  struct kvs;

  /**
   * @brief A tagged variant representing a JSON-like value.
   *
   * A token stores one of the supported primitive types, or a shared_ptr to
   * an array_t or kvs to support recursion.
   */
  struct token
  {
    /**
     * @brief The underlying variant type.
     */
    using value_t = std::variant<
        std::monostate,           // null
        bool,                     // boolean
        std::int64_t,             // integer (stored as int64)
        double,                   // floating point
        std::string,              // string
        std::shared_ptr<array_t>, // array
        std::shared_ptr<kvs>      // object
        >;

    /**
     * @brief Stored value, default is null.
     */
    value_t v{std::monostate{}};

    // Constructors

    /// @brief Construct null.
    token() = default;

    /// @brief Construct null.
    token(std::nullptr_t) : v(std::monostate{}) {}

    /// @brief Construct boolean.
    token(bool b) : v(b) {}

    /**
     * @brief Construct integer from any integral type (except bool).
     *
     * All integers are stored as int64 to keep a single integer representation.
     * Note: values outside int64 range will wrap (intentional for this minimal model).
     */
    template <class I>
      requires(std::is_integral_v<I> && !std::is_same_v<std::remove_cvref_t<I>, bool>)
    token(I i) : v(static_cast<std::int64_t>(i))
    {
    }

    /**
     * @brief Construct floating point.
     */
    token(double d) : v(d) {}

    /// @brief Construct string from C string (null pointer becomes empty string).
    token(const char *s) : v(std::string(s ? s : "")) {}

    /// @brief Construct string (moved).
    token(std::string s) : v(std::move(s)) {}

    /// @brief Construct object token from kvs (copies).
    token(const kvs &obj);

    /// @brief Construct array token from array_t (copies).
    token(const array_t &arr);

    // Type checks

    /// @brief True if token is null.
    bool is_null() const noexcept { return std::holds_alternative<std::monostate>(v); }

    /// @brief True if token is bool.
    bool is_bool() const noexcept { return std::holds_alternative<bool>(v); }

    /// @brief True if token is int64.
    bool is_i64() const noexcept { return std::holds_alternative<std::int64_t>(v); }

    /// @brief True if token is double.
    bool is_f64() const noexcept { return std::holds_alternative<double>(v); }

    /// @brief True if token is string.
    bool is_string() const noexcept { return std::holds_alternative<std::string>(v); }

    /// @brief True if token is array.
    bool is_array() const noexcept { return std::holds_alternative<std::shared_ptr<array_t>>(v); }

    /// @brief True if token is object.
    bool is_object() const noexcept { return std::holds_alternative<std::shared_ptr<kvs>>(v); }

    // Raw getters (nullptr if wrong type)

    /// @brief Get pointer to bool value, or nullptr.
    const bool *as_bool() const noexcept { return std::get_if<bool>(&v); }

    /// @brief Get pointer to int64 value, or nullptr.
    const std::int64_t *as_i64() const noexcept { return std::get_if<std::int64_t>(&v); }

    /// @brief Get pointer to double value, or nullptr.
    const double *as_f64() const noexcept { return std::get_if<double>(&v); }

    /// @brief Get pointer to string value, or nullptr.
    const std::string *as_string() const noexcept { return std::get_if<std::string>(&v); }

    /// @brief Get shared_ptr to array container, or nullptr.
    std::shared_ptr<array_t> as_array_ptr() const noexcept
    {
      if (auto p = std::get_if<std::shared_ptr<array_t>>(&v))
        return *p;
      return nullptr;
    }

    /// @brief Get shared_ptr to object container, or nullptr.
    std::shared_ptr<kvs> as_object_ptr() const noexcept
    {
      if (auto p = std::get_if<std::shared_ptr<kvs>>(&v))
        return *p;
      return nullptr;
    }

    // Convenience getters (value or default)

    /// @brief Get bool or return default.
    bool as_bool_or(bool def) const noexcept
    {
      if (auto p = as_bool())
        return *p;
      return def;
    }

    /// @brief Get int64 or return default.
    std::int64_t as_i64_or(std::int64_t def) const noexcept
    {
      if (auto p = as_i64())
        return *p;
      return def;
    }

    /// @brief Get double or return default.
    double as_f64_or(double def) const noexcept
    {
      if (auto p = as_f64())
        return *p;
      return def;
    }

    /// @brief Get string or return default.
    std::string as_string_or(std::string def) const
    {
      if (auto p = as_string())
        return *p;
      return def;
    }

    // Mutating helpers and setters

    /// @brief Set token to null.
    void set_null() { v = std::monostate{}; }

    /// @brief Set token to bool.
    void set_bool(bool b) { v = b; }

    /// @brief Set token to int64.
    void set_i64(std::int64_t x) { v = x; }

    /// @brief Set token to int (promoted to int64).
    void set_int(int x) { v = static_cast<std::int64_t>(x); }

    /// @brief Set token to long long (promoted to int64).
    void set_ll(long long x) { v = static_cast<std::int64_t>(x); }

    /// @brief Set token to unsigned long long (stored as int64).
    void set_ull(unsigned long long x) { v = static_cast<std::int64_t>(x); }

    /// @brief Set token to double.
    void set_f64(double d) { v = d; }

    /// @brief Set token to string (moved).
    void set_string(std::string s) { v = std::move(s); }

    /// @brief Set token to string from C string.
    void set_cstr(const char *s) { v = std::string(s ? s : ""); }

    /// @brief Set token to array (copies into shared_ptr).
    void set_array(const array_t &a);

    /// @brief Set token to object (copies into shared_ptr).
    void set_object(const kvs &o);

    /**
     * @brief Ensure token is an array and return a mutable reference.
     *
     * If the token is not currently an array, it is replaced by an empty array.
     */
    array_t &ensure_array();

    /**
     * @brief Ensure token is an object and return a mutable reference.
     *
     * If the token is not currently an object, it is replaced by an empty object.
     */
    kvs &ensure_object();
  };

  /**
   * @brief Token list representing a JSON array.
   *
   * This behaves like a vector<token> with convenience methods.
   */
  struct array_t
  {
    /// @brief Array elements.
    std::vector<token> elems{};

    /// @brief Construct empty array.
    array_t() = default;

    /// @brief Construct from initializer_list.
    array_t(std::initializer_list<token> l) : elems(l) {}

    /// @brief Construct by copy.
    explicit array_t(const std::vector<token> &v) : elems(v) {}

    /// @brief Construct by move.
    explicit array_t(std::vector<token> &&v) : elems(std::move(v)) {}

    /// @brief Number of elements.
    std::size_t size() const noexcept { return elems.size(); }

    /// @brief True if empty.
    bool empty() const noexcept { return elems.empty(); }

    /// @brief Clear elements.
    void clear() { elems.clear(); }

    /// @brief Reserve capacity.
    void reserve(std::size_t n) { elems.reserve(n); }

    /// @brief Current capacity.
    std::size_t capacity() const noexcept { return elems.capacity(); }

    /// @brief Direct element access (no bounds checks).
    token &operator[](std::size_t i) { return elems[i]; }

    /// @brief Direct element access (no bounds checks).
    const token &operator[](std::size_t i) const { return elems[i]; }

    /// @brief Bounds-checked access.
    token &at(std::size_t i) { return elems.at(i); }

    /// @brief Bounds-checked access.
    const token &at(std::size_t i) const { return elems.at(i); }

    /// @brief Append token.
    void push_back(token t) { elems.emplace_back(std::move(t)); }

    /// @brief Append null token.
    void push_null() { elems.emplace_back(nullptr); }

    /// @brief Append bool.
    void push_bool(bool b) { elems.emplace_back(b); }

    /// @brief Append int (promoted to int64).
    void push_int(int x) { elems.emplace_back(x); }

    /// @brief Append int64.
    void push_i64(std::int64_t x) { elems.emplace_back(x); }

    /// @brief Append long long (promoted to int64).
    void push_ll(long long x) { elems.emplace_back(static_cast<std::int64_t>(x)); }

    /// @brief Append unsigned long long (stored as int64).
    void push_ull(unsigned long long x) { elems.emplace_back(static_cast<std::int64_t>(x)); }

    /// @brief Append double.
    void push_f64(double d) { elems.emplace_back(d); }

    /// @brief Append string.
    void push_string(std::string s) { elems.emplace_back(std::move(s)); }

    /// @brief Append C string.
    void push_cstr(const char *s) { elems.emplace_back(s); }

    /// @brief Pop last element (no check).
    void pop_back() { elems.pop_back(); }

    /// @brief Resize array, new elements are null.
    void resize(std::size_t n) { elems.resize(n, token(nullptr)); }

    /// @brief Resize array, fill with provided token.
    void resize(std::size_t n, const token &fill) { elems.resize(n, fill); }

    /// @brief Iterators.
    auto begin() noexcept { return elems.begin(); }
    auto end() noexcept { return elems.end(); }
    auto begin() const noexcept { return elems.begin(); }
    auto end() const noexcept { return elems.end(); }

    /// @brief Access underlying vector.
    std::vector<token> &data() noexcept { return elems; }
    const std::vector<token> &data() const noexcept { return elems; }

    /**
     * @brief Ensure at least index+1 elements exist, filling missing with null.
     * @return Reference to element at idx.
     */
    token &ensure(std::size_t idx)
    {
      if (elems.size() <= idx)
        elems.resize(idx + 1, token(nullptr));
      return elems[idx];
    }

    /**
     * @brief Remove element at index (preserves order).
     * @return true if removed.
     */
    bool erase_at(std::size_t idx)
    {
      if (idx >= elems.size())
        return false;
      elems.erase(elems.begin() + static_cast<std::ptrdiff_t>(idx));
      return true;
    }
  };

  /**
   * @brief Flattened key/value list representing a JSON object.
   *
   * Layout:
   *   key0, value0, key1, value1, ...
   *
   * Keys are typically string tokens.
   */
  struct kvs
  {
    /// @brief Raw flat storage.
    std::vector<token> flat{};

    /// @brief Construct empty object.
    kvs() = default;

    /// @brief Construct from initializer_list (flattened key/value order).
    kvs(std::initializer_list<token> list) : flat(list) {}

    /// @brief Construct by copy.
    explicit kvs(const std::vector<token> &v) : flat(v) {}

    /// @brief Construct by move.
    explicit kvs(std::vector<token> &&v) : flat(std::move(v)) {}

    /// @brief True if object has no pairs.
    bool empty() const noexcept { return flat.empty(); }

    /// @brief Clear all pairs.
    void clear() { flat.clear(); }

    /// @brief Reserve for N pairs (reserves 2*N tokens).
    void reserve_pairs(std::size_t n_pairs) { flat.reserve(n_pairs * 2); }

    /// @brief Total raw tokens in flat storage.
    std::size_t raw_size() const noexcept { return flat.size(); }

    /// @brief Number of pairs (raw_size/2).
    std::size_t size_pairs() const noexcept { return flat.size() / 2; }

    /// @brief Current raw capacity.
    std::size_t capacity() const noexcept { return flat.capacity(); }

    /// @brief Iterators over raw tokens.
    auto begin() noexcept { return flat.begin(); }
    auto end() noexcept { return flat.end(); }
    auto begin() const noexcept { return flat.begin(); }
    auto end() const noexcept { return flat.end(); }

    /// @brief Access underlying vector.
    std::vector<token> &data() noexcept { return flat; }
    const std::vector<token> &data() const noexcept { return flat; }

    /// @brief Sentinel for "not found".
    static constexpr std::size_t npos() noexcept { return static_cast<std::size_t>(-1); }

    /// @brief Only string token is considered a valid key.
    static bool token_is_key_string(const token &t) noexcept { return t.is_string(); }

    /**
     * @brief Find the raw index of a key inside flat storage.
     *
     * @return Index i such that flat[i] is key and flat[i+1] is value, or npos().
     */
    std::size_t find_key_index(std::string_view key) const noexcept
    {
      const std::size_t n = flat.size();
      for (std::size_t i = 0; i + 1 < n; i += 2)
      {
        const token &k = flat[i];
        if (!token_is_key_string(k))
          continue;
        const std::string *ks = k.as_string();
        if (ks && *ks == key)
          return i;
      }
      return npos();
    }

    /// @brief Check if object contains key.
    bool contains(std::string_view key) const noexcept { return find_key_index(key) != npos(); }

    /// @brief Get pointer to value for a key, nullptr if missing.
    const token *get_ptr(std::string_view key) const noexcept
    {
      const std::size_t i = find_key_index(key);
      if (i == npos())
        return nullptr;
      return &flat[i + 1];
    }

    /// @brief Get pointer to value for a key, nullptr if missing.
    token *get_ptr(std::string_view key) noexcept
    {
      const std::size_t i = find_key_index(key);
      if (i == npos())
        return nullptr;
      return &flat[i + 1];
    }

    /**
     * @brief Get or create a value for key.
     *
     * If the key does not exist, it is appended with a null value.
     */
    token &operator[](std::string_view key)
    {
      if (token *p = get_ptr(key))
        return *p;

      flat.emplace_back(std::string(key));
      flat.emplace_back(nullptr);
      return flat.back();
    }

    /// @brief Set key to a token (insert or replace).
    void set(std::string_view key, token value)
    {
      if (token *p = get_ptr(key))
      {
        *p = std::move(value);
        return;
      }
      flat.emplace_back(std::string(key));
      flat.emplace_back(std::move(value));
    }

    /// @brief Append raw key/value tokens without validation.
    void push_pair(token key_token, token value_token)
    {
      flat.emplace_back(std::move(key_token));
      flat.emplace_back(std::move(value_token));
    }

    /// @brief Remove a key (preserves order).
    bool erase(std::string_view key)
    {
      const std::size_t i = find_key_index(key);
      if (i == npos())
        return false;

      flat.erase(flat.begin() + static_cast<std::ptrdiff_t>(i),
                 flat.begin() + static_cast<std::ptrdiff_t>(i + 2));
      return true;
    }

    /**
     * @brief Remove all pairs for which predicate returns true.
     *
     * Predicate: bool pred(std::string_view key, const token& value)
     */
    template <class Pred>
    std::size_t erase_if(Pred pred)
    {
      std::size_t removed = 0;
      std::vector<token> out;
      out.reserve(flat.size());

      const std::size_t n = flat.size();
      for (std::size_t i = 0; i + 1 < n; i += 2)
      {
        const token &k = flat[i];
        const token &val = flat[i + 1];

        if (!k.is_string())
        {
          out.emplace_back(k);
          out.emplace_back(val);
          continue;
        }

        const std::string *sp = k.as_string();
        if (!sp)
        {
          out.emplace_back(k);
          out.emplace_back(val);
          continue;
        }

        const std::string_view ks(*sp);
        if (pred(ks, val))
        {
          ++removed;
          continue;
        }

        out.emplace_back(k);
        out.emplace_back(val);
      }

      flat.swap(out);
      return removed;
    }

    /**
     * @brief Iterate on pairs (const).
     *
     * Skips pairs where key is not a string token.
     */
    template <class Fn>
    void for_each_pair(Fn fn) const
    {
      const std::size_t n = flat.size();
      for (std::size_t i = 0; i + 1 < n; i += 2)
      {
        const token &k = flat[i];
        if (!k.is_string())
          continue;

        const std::string *ks = k.as_string();
        if (!ks)
          continue;

        fn(std::string_view(*ks), flat[i + 1]);
      }
    }

    /**
     * @brief Iterate on pairs (mutable value).
     */
    template <class Fn>
    void for_each_pair(Fn fn)
    {
      const std::size_t n = flat.size();
      for (std::size_t i = 0; i + 1 < n; i += 2)
      {
        token &k = flat[i];
        if (!k.is_string())
          continue;

        std::string *ks = std::get_if<std::string>(&k.v);
        if (!ks)
          continue;

        fn(std::string_view(*ks), flat[i + 1]);
      }
    }

    /// @brief Return a list of keys (string copies).
    std::vector<std::string> keys() const
    {
      std::vector<std::string> out;
      out.reserve(size_pairs());
      for_each_pair([&](std::string_view k, const token &)
                    { out.emplace_back(k); });
      return out;
    }

    /**
     * @brief Merge another object into this one.
     *
     * If overwrite is true: existing keys are replaced.
     * If overwrite is false: existing keys are kept.
     */
    std::size_t merge_from(const kvs &other, bool overwrite = true)
    {
      std::size_t changes = 0;
      other.for_each_pair([&](std::string_view k, const token &v)
                          {
        if (!overwrite && contains(k))
          return;
        set(k, v);
        ++changes; });
      return changes;
    }

    // Typed getters

    std::optional<std::string> get_string(std::string_view key) const
    {
      if (const token *p = get_ptr(key))
      {
        if (auto s = p->as_string())
          return *s;
      }
      return std::nullopt;
    }

    std::optional<std::int64_t> get_i64(std::string_view key) const noexcept
    {
      if (const token *p = get_ptr(key))
      {
        if (auto x = p->as_i64())
          return *x;
      }
      return std::nullopt;
    }

    std::optional<double> get_f64(std::string_view key) const noexcept
    {
      if (const token *p = get_ptr(key))
      {
        if (auto x = p->as_f64())
          return *x;
      }
      return std::nullopt;
    }

    std::optional<bool> get_bool(std::string_view key) const noexcept
    {
      if (const token *p = get_ptr(key))
      {
        if (auto x = p->as_bool())
          return *x;
      }
      return std::nullopt;
    }

    // Typed getters with default

    std::string get_string_or(std::string_view key, std::string def) const
    {
      if (auto v = get_string(key))
        return *v;
      return def;
    }

    std::int64_t get_i64_or(std::string_view key, std::int64_t def) const noexcept
    {
      if (auto v = get_i64(key))
        return *v;
      return def;
    }

    double get_f64_or(std::string_view key, double def) const noexcept
    {
      if (auto v = get_f64(key))
        return *v;
      return def;
    }

    bool get_bool_or(std::string_view key, bool def) const noexcept
    {
      if (auto v = get_bool(key))
        return *v;
      return def;
    }

    std::string get_string_or(std::string_view key) const
    {
      return get_string_or(key, std::string{});
    }

    std::int64_t get_i64_or(std::string_view key) const noexcept
    {
      return get_i64_or(key, 0);
    }

    double get_f64_or(std::string_view key) const noexcept
    {
      return get_f64_or(key, 0.0);
    }

    bool get_bool_or(std::string_view key) const noexcept
    {
      return get_bool_or(key, false);
    }

    // Typed setters

    void set_string(std::string_view key, std::string v) { set(key, token(std::move(v))); }
    void set_bool(std::string_view key, bool v) { set(key, token(v)); }
    void set_f64(std::string_view key, double v) { set(key, token(v)); }

    /// @brief Set integer value from int64.
    void set_i64(std::string_view key, std::int64_t v) { set(key, token(v)); }

    /// @brief Set integer value from int (promoted to int64).
    void set_int(std::string_view key, int v) { set(key, token(v)); }

    /// @brief Set integer value from long long (promoted to int64).
    void set_ll(std::string_view key, long long v)
    {
      set(key, token(static_cast<std::int64_t>(v)));
    }

    void set_ull(std::string_view key, unsigned long long v)
    {
      set(key, token(static_cast<std::int64_t>(v)));
    }

    /// @brief Ensure value is an object and return it.
    kvs &ensure_object(std::string_view key)
    {
      token &t = (*this)[key];
      return t.ensure_object();
    }

    /// @brief Ensure value is an array and return it.
    array_t &ensure_array(std::string_view key)
    {
      token &t = (*this)[key];
      return t.ensure_array();
    }
  };

  // token conversions

  inline token::token(const kvs &obj) : v(std::make_shared<kvs>(obj)) {}
  inline token::token(const array_t &arr) : v(std::make_shared<array_t>(arr)) {}

  inline void token::set_array(const array_t &a) { v = std::make_shared<array_t>(a); }
  inline void token::set_object(const kvs &o) { v = std::make_shared<kvs>(o); }

  inline array_t &token::ensure_array()
  {
    if (!is_array())
      v = std::make_shared<array_t>();
    return *std::get<std::shared_ptr<array_t>>(v);
  }

  inline kvs &token::ensure_object()
  {
    if (!is_object())
      v = std::make_shared<kvs>();
    return *std::get<std::shared_ptr<kvs>>(v);
  }

  // Builders (existing API)

  /// @brief Build a Simple array from initializer_list.
  inline array_t array(std::initializer_list<token> l) { return array_t{l}; }

  /// @brief Build a Simple object from initializer_list (flattened key/value order).
  inline kvs obj(std::initializer_list<token> l) { return kvs{l}; }

  /// @brief Build a Simple array from vector (copy).
  inline array_t array(const std::vector<token> &v) { return array_t{v}; }

  /// @brief Build a Simple array from vector (move).
  inline array_t array(std::vector<token> &&v) { return array_t{std::move(v)}; }

  /// @brief Build a Simple object from vector (copy).
  inline kvs obj(const std::vector<token> &v) { return kvs{v}; }

  /// @brief Build a Simple object from vector (move).
  inline kvs obj(std::vector<token> &&v) { return kvs{std::move(v)}; }

  /**
   * @brief Explicit aliases to reduce ambiguity when also using <vix/json.hpp>.
   *
   * They call the same builders and do not change behavior.
   */
  inline kvs simple_obj(std::initializer_list<token> l) { return obj(l); }
  inline array_t simple_array(std::initializer_list<token> l) { return array(l); }
  inline kvs simple_obj(const std::vector<token> &v) { return obj(v); }
  inline kvs simple_obj(std::vector<token> &&v) { return obj(std::move(v)); }
  inline array_t simple_array(const std::vector<token> &v) { return array(v); }
  inline array_t simple_array(std::vector<token> &&v) { return array(std::move(v)); }

} // namespace vix::json

#endif // VIX_JSON_SIMPLE_HPP
