#ifndef VIX_BUILD_HPP
#define VIX_BUILD_HPP

#include <nlohmann/json.hpp>
#include <utility>
#include <string_view>

namespace Vix::json
{
    using Json = nlohmann::json;

    inline void _put(Json &) {}
    template <class K, class V, class... Rest>
    inline void _put(Json &j, K &&k, V &&v, Rest &&...rest)
    {
        j[std::forward<K>(k)] = std::forward<V>(v);
        if constexpr (sizeof...(rest) > 0)
            _put(j, std::forward<Rest>(rest)...);
    }

    template <class... Args>
    inline Json o(Args &&...args)
    {
        static_assert(sizeof...(args) % 2 == 0,
                      "json::o requires (k1,v1,k2,v2,...)");
        Json j = Json::object();
        if constexpr (sizeof...(args) > 0)
            _put(j, std::forward<Args>(args)...);
        return j;
    }

    template <class... Ts>
    inline Json a(Ts &&...ts)
    {
        return Json::array({std::forward<Ts>(ts)...});
    }

    inline Json kv(std::initializer_list<std::pair<std::string, Json>> xs)
    {
        Json j = Json::object();
        for (auto &[k, v] : xs)
            j[k] = v;
        return j;
    }
}

#endif