#ifndef VIX_CONVERT_HPP
#define VIX_CONVERT_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace Vix::json
{
    using Json = nlohmann::json;

    template <class T>
    inline std::optional<T> get_opt(const Json *j)
    {
        if (!j)
            return std::nullopt;
        try
        {
            return j->get<T>();
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    template <class T>
    inline T get_or(const Json *j, T def)
    {
        if (!j)
            return def;
        try
        {
            return j->get<T>();
        }
        catch (...)
        {
            return def;
        }
    }

    template <class T>
    inline T ensure(const Json &j)
    {
        return j.get<T>();
    }

}

#endif