#ifndef VIX_JPATH_HPP
#define VIX_JPATH_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <cstddef>
#include <charconv>

/**
 * @file VIX_JPATH_HPP
 * @brief JSON path navigation and mutation utilities for *Vix.cpp*.
 *
 * This header provides a minimal **JPath** implementation to traverse and modify
 * JSON documents using string paths similar to JavaScript-style expressions.
 *
 * ### Supported Syntax
 * - Dot notation: `"user.name.first"`
 * - Array indices: `"users[0].email"`
 * - Quoted keys with brackets: `["complex.key"].value`
 *
 * ### Example
 * @code
 * using namespace Vix::json;
 *
 * Json j = R"({
 *   "user": { "name": "Ada", "roles": ["admin", "editor"] },
 *   "settings": { "theme": "dark" }
 * })"_json;
 *
 * const Json* theme = jget(j, "settings.theme");
 * std::cout << "theme=" << (theme ? theme->get<std::string>() : "N/A") << "\\n";
 *
 * // Modify
 * jset(j, "user.roles[1]", "developer");
 * jset(j, "user.address.city", "Kampala");
 *
 * std::cout << j.dump(2) << "\\n";
 * @endcode
 */

namespace Vix::json
{
    /// Alias utilitaire pour `nlohmann::json`.
    using Json = nlohmann::json;

    /**
     * @brief Token struct representing a parsed path segment (key or index).
     */
    struct Token
    {
        /// Kind of token.
        enum Kind
        {
            Key,  ///< Object key (string)
            Index ///< Array index (number)
        } kind{};

        /// Key name (used if kind == Key).
        std::string key;

        /// Array index (used if kind == Index).
        std::size_t index = static_cast<std::size_t>(-1);
    };

    // ---------------------------------------------------------------------
    // Internal parsing utilities
    // ---------------------------------------------------------------------
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
         * @brief Parse a quoted key inside brackets: ["key name"]
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
         * @brief Parse a full JPath into tokens, without throwing.
         *
         * @param path JPath string (e.g. `"user.roles[0].name"`).
         * @param out  Vector to store parsed tokens.
         * @param err  Error message in case of failure.
         * @return true on success, false otherwise.
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

    // ---------------------------------------------------------------------
    // Public API
    // ---------------------------------------------------------------------

    /**
     * @brief Tokenize a JPath string into structured tokens (throws on error).
     *
     * @param path JPath expression.
     * @return Vector of parsed tokens.
     * @throws std::runtime_error on invalid syntax.
     */
    inline std::vector<Token> tokenize_path(std::string_view path)
    {
        std::vector<Token> out;
        std::string err;
        if (!detail::tokenize_path_nothrow(path, out, err))
            throw std::runtime_error(err);
        return out;
    }

    // ---------------------------------------------------------------------
    // jget — read-only JSON pointer
    // ---------------------------------------------------------------------

    /**
     * @brief Safely navigate a JSON tree using a JPath string.
     *
     * @param j JSON root.
     * @param path JPath expression (e.g. `"users[0].name"`).
     * @return Pointer to the JSON node, or `nullptr` if missing/invalid.
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

    // ---------------------------------------------------------------------
    // jget — mutable variant (creates intermediate nodes)
    // ---------------------------------------------------------------------

    /**
     * @brief Obtain a mutable reference to a node, creating objects/arrays as needed.
     *
     * Missing keys or array slots are automatically created as `null` or empty containers.
     *
     * @param j JSON root.
     * @param path JPath expression.
     * @return Pointer to the created or existing JSON node.
     * @throws std::runtime_error on syntax error.
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

    // ---------------------------------------------------------------------
    // jset — convenience mutator
    // ---------------------------------------------------------------------

    /**
     * @brief Set a JSON value at the specified path (auto-creates intermediate nodes).
     *
     * @tparam T Value type (convertible to JSON).
     * @param j JSON root to modify.
     * @param path JPath expression.
     * @param v Value to assign.
     * @return true if successful, false if a parsing or type error occurred.
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

} // namespace Vix::json

#endif // VIX_JPATH_HPP
