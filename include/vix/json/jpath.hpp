/**
 *
 *  @file jpath.hpp
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
#ifndef VIX_JSON_JPATH_HPP
#define VIX_JSON_JPATH_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <cstddef>
#include <charconv>

/**
 * @file jpath.hpp
 * @brief Navigate and mutate JSON using a small path language (JPath).
 *
 * @details
 * This header provides a minimal, dependency-free path syntax to:
 * - Read deeply nested values (without chaining many `[]` calls)
 * - Create missing objects/arrays automatically for writes
 *
 * It is useful for:
 * - Configuration trees
 * - Dynamic payload building
 * - Adapters and bridges between modules
 *
 * ---
 *
 * ## Supported syntax (beginner friendly)
 * 1) Dot notation:
 *    - `"user.name"`
 *    - `"settings.theme"`
 *
 * 2) Array indices:
 *    - `"users[0].email"`
 *    - `"roles[1]"`
 *
 * 3) Quoted keys inside brackets:
 *    - `["complex.key"].value`
 *    - `["a b c"][0]`
 *
 * ---
 *
 * ## Read vs Write behavior (important)
 *
 * - `const Json* jget(const Json&, path)`:
 *   - Never throws
 *   - Returns nullptr if path is invalid or missing
 *
 * - `Json* jget(Json&, path)`:
 *   - May throw on syntax errors
 *   - Automatically creates missing intermediate nodes:
 *     - missing object keys become created objects
 *     - missing array indices grow the array and fill with nulls
 *
 * - `bool jset(Json&, path, value)`:
 *   - Calls writable `jget` internally
 *   - Returns false if syntax is invalid or assignment fails
 *
 * ---
 *
 * ## Example
 * @code
 * using namespace vix::json;
 *
 * Json j = R"({
 *   "user": { "name": "Ada", "roles": ["admin", "editor"] },
 *   "settings": { "theme": "dark" }
 * })"_json;
 *
 * // Read
 * if (const Json* theme = jget(j, "settings.theme")) {
 *   std::cout << "theme=" << theme->get<std::string>() << "\n";
 * }
 *
 * // Write (auto-creates)
 * jset(j, "user.roles[1]", "developer");
 * jset(j, "user.address.city", "Kampala");
 *
 * std::cout << j.dump(2) << "\n";
 * @endcode
 *
 * ---
 *
 * ## Expert notes
 * - Path tokenization is done with a lightweight parser (no regex).
 * - Indices are parsed with `std::from_chars` for speed and no locale impact.
 * - This is not a full JSONPath implementation: it is intentionally small.
 */

namespace vix::json
{
  /// Primary JSON type used across Vix.cpp.
  using Json = nlohmann::json;

  /**
   * @brief A parsed path segment (either object key or array index).
   *
   * This is the output of tokenize_path() and is used internally by jget/jset.
   */
  struct Token
  {
    /// Token kind (object key or array index).
    enum Kind
    {
      Key,  ///< Object key segment
      Index ///< Array index segment
    } kind{};

    /// Key name (used when kind == Key).
    std::string key;

    /// Index (used when kind == Index).
    std::size_t index = static_cast<std::size_t>(-1);
  };

  namespace detail
  {
    inline void skip_spaces(std::string_view s, std::size_t &i) noexcept
    {
      while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])))
        ++i;
    }

    inline std::size_t count_segments(std::string_view path) noexcept
    {
      std::size_t c = 1;
      for (char ch : path)
        if (ch == '.' || ch == '[')
          ++c;
      return c;
    }

    inline bool parse_index(std::string_view s, std::size_t &out)
    {
      std::size_t i = 0, n = s.size();
      skip_spaces(s, i);
      if (i >= n)
        return false;

      const char *first = s.data() + i;
      const char *last = s.data() + n;

      if (*first == '+' || *first == '-') // indices not signed
        return false;

      unsigned long long tmp = 0;
      auto [ptr, ec] = std::from_chars(first, last, tmp, 10);
      if (ec != std::errc{})
        return false;

      while (ptr < last && std::isspace(static_cast<unsigned char>(*ptr)))
        ++ptr;
      if (ptr != last)
        return false;

      out = static_cast<std::size_t>(tmp);
      return true;
    }

    /**
     * @brief Parse a quoted key inside brackets: ["key name"].
     *
     * Supports basic escaping: \" and \\.
     *
     * @param path Full path string
     * @param i Current offset (points to '['), updated on success
     * @param out_key Parsed key output
     * @param err Error string on failure
     * @return true on success, false on error
     */
    inline bool parse_bracket_string_key(std::string_view path, std::size_t &i,
                                         std::string &out_key, std::string &err)
    {
      const std::size_t n = path.size();
      ++i; // skip '['
      skip_spaces(path, i);
      if (i >= n || path[i] != '"')
      {
        err = "Invalid jpath: expected '\"' after '[' for quoted key";
        return false;
      }

      ++i; // skip first '"'
      out_key.clear();
      while (i < n)
      {
        char ch = path[i++];
        if (ch == '\\')
        {
          if (i >= n)
          {
            err = "Invalid jpath: dangling escape in quoted key";
            return false;
          }
          char esc = path[i++];
          switch (esc)
          {
          case '\\':
            out_key.push_back('\\');
            break;
          case '"':
            out_key.push_back('"');
            break;
          default:
            out_key.push_back(esc);
            break;
          }
        }
        else if (ch == '"')
        {
          break;
        }
        else
        {
          out_key.push_back(ch);
        }
      }
      if (i > n || path[i - 1] != '"')
      {
        err = "Invalid jpath: missing closing '\"' in quoted key";
        return false;
      }

      skip_spaces(path, i);
      if (i >= n || path[i] != ']')
      {
        err = "Invalid jpath: missing ']' after quoted key";
        return false;
      }
      ++i; // skip ']'
      return true;
    }

    /**
     * @brief Parse a full JPath into tokens (no throw).
     *
     * @param path JPath string, e.g. "user.roles[0].name"
     * @param out Output tokens
     * @param err Human-readable error message on failure
     * @return true on success, false on error
     */
    inline bool tokenize_path_nothrow(std::string_view path, std::vector<Token> &out, std::string &err)
    {
      out.clear();
      out.reserve(count_segments(path));

      std::string cur;
      cur.reserve(32);

      const std::size_t n = path.size();
      std::size_t i = 0;

      auto flush_key = [&]() -> bool
      {
        if (!cur.empty())
        {
          out.push_back(Token{Token::Key, cur, static_cast<std::size_t>(-1)});
          cur.clear();
          return true;
        }
        return false;
      };

      while (i < n)
      {
        char ch = path[i];

        if (ch == '.')
        {
          if (!flush_key())
          {
            err = "Invalid jpath: empty key segment at offset " + std::to_string(i);
            return false;
          }
          ++i;
          continue;
        }

        if (ch == '[')
        {
          std::size_t look = i + 1;
          skip_spaces(path, look);
          if (look < n && path[look] == '"')
          {
            if (!flush_key() && !cur.empty())
            {
              err = "Invalid jpath: unexpected state before quoted key";
              return false;
            }
            std::string k;
            if (!parse_bracket_string_key(path, i, k, err))
              return false;
            out.push_back(Token{Token::Key, std::move(k), static_cast<std::size_t>(-1)});
            continue;
          }

          if (!cur.empty())
          {
            out.push_back(Token{Token::Key, cur, static_cast<std::size_t>(-1)});
            cur.clear();
          }
          ++i; // skip '['
          std::size_t start = i;
          while (i < n && path[i] != ']')
            ++i;
          if (i >= n)
          {
            err = "Invalid jpath: missing closing ']'";
            return false;
          }

          std::string_view inside{path.data() + start, i - start};
          ++i; // skip ']'

          std::size_t idx = 0;
          if (!parse_index(inside, idx))
          {
            err = "Invalid jpath: bad array index inside []";
            return false;
          }
          out.push_back(Token{Token::Index, {}, idx});
          continue;
        }

        cur.push_back(ch);
        ++i;
      }

      if (!cur.empty())
        out.push_back(Token{Token::Key, cur, static_cast<std::size_t>(-1)});

      return true;
    }
  } // namespace detail

  /**
   * @brief Tokenize a JPath string into structured tokens (throws on error).
   *
   * @param path JPath expression
   * @return Parsed tokens
   * @throws std::runtime_error on invalid syntax
   */
  inline std::vector<Token> tokenize_path(std::string_view path)
  {
    std::vector<Token> out;
    std::string err;
    if (!detail::tokenize_path_nothrow(path, out, err))
      throw std::runtime_error(err);
    return out;
  }

  /**
   * @brief Read-only navigation: returns pointer or nullptr.
   *
   * @param j JSON root
   * @param path JPath expression, e.g. "users[0].name"
   * @return Pointer to node, or nullptr if missing or invalid
   *
   * @note Never throws.
   */
  inline const Json *jget(const Json &j, std::string_view path)
  {
    const Json *cur = &j;
    std::string err;
    std::vector<Token> toks;
    if (!detail::tokenize_path_nothrow(path, toks, err))
      return nullptr;

    for (const auto &t : toks)
    {
      if (t.kind == Token::Key)
      {
        if (!cur->is_object())
          return nullptr;
        auto it = cur->find(t.key);
        if (it == cur->end())
          return nullptr;
        cur = &(*it);
      }
      else
      {
        if (!cur->is_array() || t.index >= cur->size())
          return nullptr;
        cur = &((*cur)[t.index]);
      }
    }
    return cur;
  }

  /// @overload for const char* paths.
  inline const Json *jget(const Json &j, const char *path)
  {
    return jget(j, std::string_view{path});
  }

  /**
   * @brief Writable navigation: returns pointer, creates missing nodes.
   *
   * @details
   * Intermediate nodes are created as needed:
   * - Keys create objects (`{}`)
   * - Indices create arrays (`[]`) and expand with nulls
   *
   * @param j JSON root
   * @param path JPath expression
   * @return Pointer to the created or existing node
   * @throws std::runtime_error on invalid syntax
   */
  inline Json *jget(Json &j, std::string_view path)
  {
    Json *cur = &j;
    std::string err;
    std::vector<Token> toks;
    if (!detail::tokenize_path_nothrow(path, toks, err))
      throw std::runtime_error(err);

    for (const auto &t : toks)
    {
      if (t.kind == Token::Key)
      {
        if (!cur->is_object())
          *cur = Json::object();
        cur = &((*cur)[t.key]);
      }
      else
      {
        if (!cur->is_array())
          *cur = Json::array();
        while (cur->size() <= t.index)
          cur->push_back(nullptr);
        cur = &((*cur)[t.index]);
      }
    }
    return cur;
  }

  /// @overload for const char* paths.
  inline Json *jget(Json &j, const char *path)
  {
    return jget(j, std::string_view{path});
  }

  /**
   * @brief Assign a value at the specified path (auto-creates intermediate nodes).
   *
   * @tparam T Value type convertible to Json
   * @param j JSON root
   * @param path JPath expression
   * @param v Value to assign
   * @return true on success, false on syntax/assignment error
   *
   * @note Never throws (errors are converted to false).
   */
  template <class T>
  inline bool jset(Json &j, std::string_view path, T &&v)
  {
    try
    {
      *jget(j, path) = std::forward<T>(v);
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  /// @overload for const char* paths.
  template <class T>
  inline bool jset(Json &j, const char *path, T &&v)
  {
    return jset(j, std::string_view{path}, std::forward<T>(v));
  }

} // namespace vix::json

#endif // VIX_JSON_JPATH_HPP
