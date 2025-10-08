#ifndef VIX_BUILD_HPP
#define VIX_BUILD_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace Vix::json
{
    using Json = nlohmann::json;

    // ---------- Détails internes ----------
    namespace detail
    {
        template <class K>
        using is_key_like = std::bool_constant<std::is_convertible_v<K, std::string_view>>;

        inline void put_pairs(Json &) {}

        template <class K, class V, class... Rest,
                  std::enable_if_t<is_key_like<K>::value, int> = 0>
        inline void put_pairs(Json &j, K &&k, V &&v, Rest &&...rest)
        {
            // on force la clé en string pour éviter toute ambiguïté
            const std::string key{std::string_view(std::forward<K>(k))};
            j.emplace(key, Json(std::forward<V>(v)));
            if constexpr (sizeof...(rest) > 0)
                put_pairs(j, std::forward<Rest>(rest)...);
        }
    } // namespace detail

    // ---------- Objet: o(k1,v1,k2,v2,...) ----------
    template <class... Args>
    inline Json o(Args &&...args)
    {
        static_assert(sizeof...(args) % 2 == 0,
                      "json::o requires an even number of args: (k1,v1,k2,v2,...)");
        Json j = Json::object();
        if constexpr (sizeof...(args) > 0)
            detail::put_pairs(j, std::forward<Args>(args)...);
        return j;
    }

    // ---------- Tableau: a(v1, v2, v3, ...) ----------
    template <class... Ts>
    inline Json a(Ts &&...ts)
    {
        Json arr = Json::array();
        (arr.push_back(Json(std::forward<Ts>(ts))), ...);
        return arr;
    }

    // ---------- Objet depuis une liste clé/valeur ----------
    inline Json kv(std::initializer_list<std::pair<std::string_view, Json>> xs)
    {
        Json j = Json::object();
        for (const auto &p : xs)
            j.emplace(std::string(p.first), p.second);
        return j;
    }

} // namespace Vix::json

#endif // VIX_BUILD_HPP
