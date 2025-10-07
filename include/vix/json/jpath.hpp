#ifndef VIX_JPATH_HPP
#define VIX_JPATH_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>

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
        int index = -1;
    };

    inline std::vector<Token> tokenize_path(const std::string &path)
    {
        std::vector<Token> out;
        std::string cur;
        for (size_t i = 0; i < path.size();)
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
                while (i < path.size() && std::isdigit((unsigned char)path[i]))
                    num.push_back(path[i++]);
                if (i >= path.size() || path[i] != ']')
                    throw std::runtime_error("Invalid jpath: missing ]");
                ++i;
                out.push_back({Token::Index, {}, std::stoi(num)});
                continue;
            }
            cur.push_back(path[i++]);
        }
        if (!cur.empty())
            out.push_back({Token::Key, cur, -1});
        return out;
    }

    inline const Json *jget(const Json &j, const std::string &path)
    {
        const Json *cur = &j;
        for (auto &t : tokenize_path(path))
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
                if (t.index < 0 || t.index >= (int)cur->size())
                    return nullptr;
                cur = &((*cur)[t.index]);
            }
        }
        return cur;
    }

    inline Json *jget(Json &j, const std::string &path)
    {
        Json *cur = &j;
        for (auto &t : tokenize_path(path))
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
                if (t.index < 0)
                    throw std::runtime_error("Negative index");

                while (static_cast<int>(cur->size()) <= t.index)
                {
                    cur->push_back(nullptr);
                }
                cur = &((*cur)[static_cast<size_t>(t.index)]);
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

}

#endif
