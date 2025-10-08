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

namespace Vix::json
{
    using Json = nlohmann::json;

    struct Token
    {
        enum Kind
        {
            Key,
            Index
        } kind{};
        std::string key;
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

            if (*first == '+' || *first == '-') // pas d’index signés
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

        // Parse ["..."] (clé littérale)
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
                        break; // littéral
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
                    // cas ["key"]
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

                    // sinon: index [ 123 ]
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

                // caractère normal de clé non-quotée
                cur.push_back(ch);
                ++i;
            }

            if (!cur.empty())
                out.push_back(Token{Token::Key, cur, static_cast<std::size_t>(-1)});

            return true;
        }
    } // namespace detail

    // ---------- Tokenize (throws) ----------
    inline std::vector<Token> tokenize_path(std::string_view path)
    {
        std::vector<Token> out;
        std::string err;
        if (!detail::tokenize_path_nothrow(path, out, err))
            throw std::runtime_error(err);
        return out;
    }

    // ---------- Lecture (const) ----------
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
                if (!cur->is_array())
                    return nullptr;
                if (t.index >= cur->size())
                    return nullptr;
                cur = &((*cur)[t.index]);
            }
        }
        return cur;
    }

    // Wrappers const char* (pas de surcharge std::string pour éviter l’ambiguïté)
    inline const Json *jget(const Json &j, const char *path) { return jget(j, std::string_view{path}); }

    // ---------- Mutateur (création auto) ----------
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

    inline Json *jget(Json &j, const char *path) { return jget(j, std::string_view{path}); }

    // ---------- jset ----------
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

    template <class T>
    inline bool jset(Json &j, const char *path, T &&v)
    {
        return jset(j, std::string_view{path}, std::forward<T>(v));
    }

} // namespace Vix::json

#endif // VIX_JPATH_HPP
