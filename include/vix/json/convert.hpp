#ifndef VIX_CONVERT_HPP
#define VIX_CONVERT_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <stdexcept>

namespace Vix::json
{
    using Json = nlohmann::json;

    // ------------------- pointeurs utilitaires -------------------
    inline const Json *ptr(const Json &j, std::string_view key) noexcept
    {
        if (!j.is_object())
            return nullptr;
        auto it = j.find(std::string(key));
        return (it == j.end()) ? nullptr : &(*it);
    }

    inline const Json *ptr(const Json &j, std::size_t idx) noexcept
    {
        if (!j.is_array())
            return nullptr;
        return (idx < j.size()) ? &j[idx] : nullptr;
    }

    // ------------------- get_opt -------------------
    template <class T>
    inline std::optional<T> get_opt(const Json &j) noexcept
    {
        try
        {
            if (j.is_discarded() || j.is_null())
                return std::nullopt;
            return j.get<T>();
        }
        catch (const nlohmann::json::exception &)
        {
            return std::nullopt;
        }
    }

    template <class T>
    inline std::optional<T> get_opt(const Json *jp) noexcept
    {
        return (jp ? get_opt<T>(*jp) : std::optional<T>{});
    }

    template <class T>
    inline std::optional<T> get_opt(const Json &obj, std::string_view key) noexcept
    {
        return get_opt<T>(ptr(obj, key));
    }

    template <class T>
    inline std::optional<T> get_opt(const Json &arr, std::size_t idx) noexcept
    {
        return get_opt<T>(ptr(arr, idx));
    }

    // ------------------- get_or -------------------
    template <class T>
    inline T get_or(const Json &j, T def) noexcept
    {
        auto v = get_opt<T>(j);
        return v ? std::move(*v) : std::move(def);
    }

    template <class T>
    inline T get_or(const Json *jp, T def) noexcept
    {
        return jp ? get_or<T>(*jp, std::move(def)) : std::move(def);
    }

    template <class T>
    inline T get_or(const Json &obj, std::string_view key, T def) noexcept
    {
        auto v = get_opt<T>(obj, key);
        return v ? std::move(*v) : std::move(def);
    }

    template <class T>
    inline T get_or(const Json &arr, std::size_t idx, T def) noexcept
    {
        auto v = get_opt<T>(arr, idx);
        return v ? std::move(*v) : std::move(def);
    }

    // ------------------- ensure (throws) -------------------
    template <class T>
    inline T ensure(const Json &j)
    {
        // Laisse propager l'exception nlohmann::json::exception (type mismatch, etc.)
        return j.get<T>();
    }

    template <class T>
    inline T ensure(const Json &obj, std::string_view key)
    {
        if (!obj.is_object())
            throw std::runtime_error("ensure: not an object");

        auto it = obj.find(std::string(key));
        if (it == obj.end())
            throw std::runtime_error(std::string("ensure: missing key '") + std::string(key) + "'");

        try
        {
            return it->get<T>();
        }
        catch (const nlohmann::json::exception &e)
        {
            throw std::runtime_error(std::string("ensure: type error for key '") + std::string(key) + "': " + e.what());
        }
    }

} // namespace Vix::json

#endif // VIX_CONVERT_HPP
