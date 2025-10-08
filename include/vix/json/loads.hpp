#ifndef VIX_LOADS_HPP
#define VIX_LOADS_HPP

#include <nlohmann/json.hpp>
#include <string_view>
#include <optional>
#include <fstream>
#include <filesystem>
#include <stdexcept> // std::runtime_error
#include <string>    // pour construire des messages d'erreur

namespace Vix::json
{
    using Json = nlohmann::json;
    namespace fs = std::filesystem;

    // -------- parse depuis une string_view (throws en cas d'erreur) --------
    inline Json loads(std::string_view s)
    {
        // laisse nlohmann::json lancer ses exceptions de parsing
        return Json::parse(s);
    }

    // -------- parse (noexcept) -> optional --------
    inline std::optional<Json> try_loads(std::string_view s) noexcept
    {
        try
        {
            return Json::parse(s);
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    // -------- lecture + parse d'un fichier (throws) --------
    inline Json load_file(const fs::path &path)
    {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs)
            throw std::runtime_error("Cannot open JSON file: " + path.string());

        // lire le fichier entier
        std::string buf;
        ifs.seekg(0, std::ios::end);
        const std::streampos sz = ifs.tellg();
        if (sz > 0)
        {
            buf.resize(static_cast<std::size_t>(sz));
            ifs.seekg(0, std::ios::beg);
            ifs.read(buf.data(), sz);
        }

        return Json::parse(buf); // lève en cas d'erreur JSON
    }

    // -------- lecture + parse d'un fichier (noexcept) --------
    inline std::optional<Json> try_load_file(const fs::path &path) noexcept
    {
        try
        {
            return load_file(path);
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    // -------- wrappers const char* (évite les ambiguïtés avec std::string) --------
    inline Json loads(const char *s) { return loads(std::string_view{s}); }
    inline std::optional<Json> try_loads(const char *s) noexcept { return try_loads(std::string_view{s}); }

    inline Json load_file(const char *path) { return load_file(fs::path{path}); }
    inline std::optional<Json> try_load_file(const char *path) noexcept { return try_load_file(fs::path{path}); }

} // namespace Vix::json

#endif // VIX_LOADS_HPP
