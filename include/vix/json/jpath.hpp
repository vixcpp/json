#ifndef VIX_JPATH_HPP
#define VIX_JPATH_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <cstddef>
#include <limits>

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
        int index = -1; // we keep int to detect negative indexes, but we cast to size_t if necessary
    };

    inline std::vector<Token> tokenize_path(const std::string &path)
    {
        std::vector<Token> out;
        std::string cur;

        for (std::size_t i = 0; i < path.size();)
        {
            if (path[i] == '.')
            {
                if (!cur.empty())
                {
                    out.push_back({Token::Key, cur, -1});
                    cur.clear();
                }
                ++i;
                continue;
            }

            if (path[i] == '[')
            {
                if (!cur.empty())
                {
                    out.push_back({Token::Key, cur, -1});
                    cur.clear();
                }
                ++i;

                std::string num;
                while (i < path.size() && std::isdigit(static_cast<unsigned char>(path[i])))
                    num.push_back(path[i++]);

                if (num.empty())
                    throw std::runtime_error("Invalid jpath: empty index []");

                if (i >= path.size() || path[i] != ']')
                    throw std::runtime_error("Invalid jpath: missing ]");

                ++i;

                int idx = std::stoi(num);
                if (idx < 0)
                    throw std::runtime_error("Invalid jpath: negative index");

                out.push_back({Token::Index, {}, idx});
                continue;
            }

            cur.push_back(path[i++]);
        }

        if (!cur.empty())
            out.push_back({Token::Key, cur, -1});

        return out;
    }

    // Lecture (const)
    inline const Json *jget(const Json &j, const std::string &path)
    {
        const Json *cur = &j;
        for (const auto &t : tokenize_path(path))
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
            else // Index
            {
                if (!cur->is_array())
                    return nullptr;

                if (t.index < 0)
                    return nullptr;

                const std::size_t idx = static_cast<std::size_t>(t.index);
                if (idx >= cur->size())
                    return nullptr;

                cur = &((*cur)[idx]); // cast explicite
            }
        }
        return cur;
    }

    // Mutator access (creates missing objects/arrays)
    inline Json *jget(Json &j, const std::string &path)
    {
        Json *cur = &j;
        for (const auto &t : tokenize_path(path))
        {
            if (t.kind == Token::Key)
            {
                if (!cur->is_object())
                    *cur = Json::object();
                cur = &((*cur)[t.key]);
            }
            else // Index
            {
                if (!cur->is_array())
                    *cur = Json::array();

                if (t.index < 0)
                    throw std::runtime_error("Negative index");

                const std::size_t idx = static_cast<std::size_t>(t.index);
                while (cur->size() <= idx)
                    cur->push_back(nullptr);

                cur = &((*cur)[idx]); // idx to size_t
            }
        }
        return cur;
    }

    template <class T>
    inline bool jset(Json &j, const std::string &path, T &&v)
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

} // namespace Vix::json

#endif
