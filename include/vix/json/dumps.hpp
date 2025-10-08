#ifndef VIX_DUMPS_HPP
#define VIX_DUMPS_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <system_error>

namespace Vix::json
{
    using Json = nlohmann::json;
    namespace fs = std::filesystem;

    // -------- stringify --------
    inline std::string dumps(const Json &j, int indent = 2, bool ensure_ascii = false)
    {
        // dump(indent, indent_char, ensure_ascii, error_handler)
        return j.dump(indent, ' ', ensure_ascii);
    }

    inline std::string dumps_compact(const Json &j, bool ensure_ascii = false)
    {
        return j.dump(-1, ' ', ensure_ascii); // -1 = compact
    }

    // Alias pratique
    inline std::string dumps_pretty(const Json &j, int indent = 2, bool ensure_ascii = false)
    {
        return dumps(j, indent, ensure_ascii);
    }

    // -------- write to file (quasi-atomique) --------
    inline void dump_file(const fs::path &path, const Json &j, int indent = 2, bool ensure_ascii = false)
    {
        // 1) Prépare le répertoire
        if (path.has_parent_path())
        {
            std::error_code ec;
            fs::create_directories(path.parent_path(), ec); // best-effort
        }

        // 2) Écrit dans un fichier temporaire (même dossier)
        const fs::path tmp = path.string() + ".tmp";
        {
            std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
            if (!ofs)
                throw std::runtime_error("Cannot open temp file for writing: " + tmp.string());

            // activer exceptions sur le flux
            ofs.exceptions(std::ofstream::badbit | std::ofstream::failbit);

            try
            {
                const auto s = j.dump(indent, ' ', ensure_ascii);
                ofs.write(s.data(), static_cast<std::streamsize>(s.size()));
                ofs.flush();
                if (!ofs)
                    throw std::runtime_error("Failed to flush JSON to temp file: " + tmp.string());
            }
            catch (...)
            {
                // Nettoyage best-effort
                std::error_code ec;
                fs::remove(tmp, ec);
                throw;
            }
        }

        // 3) Remplace la destination
        std::error_code ec;
        if (fs::exists(path, ec))
        {
            fs::remove(path, ec); // best-effort
        }
        fs::rename(tmp, path, ec);
        if (ec)
        {
            // fallback : copie puis suppression
            std::error_code ec2;
            fs::copy_file(tmp, path, fs::copy_options::overwrite_existing, ec2);
            fs::remove(tmp, ec2);
            if (ec2)
                throw std::runtime_error("Failed to move JSON temp file to destination: " + path.string());
        }
    }

    // Wrappers pratiques (pas d’ambiguïté avec Loads.hpp)
    inline void dump_file(const char *path, const Json &j, int indent = 2, bool ensure_ascii = false)
    {
        dump_file(fs::path{path}, j, indent, ensure_ascii);
    }
    inline void dump_file(const std::string &path, const Json &j, int indent = 2, bool ensure_ascii = false)
    {
        dump_file(fs::path{path}, j, indent, ensure_ascii);
    }

} // namespace Vix::json

#endif // VIX_DUMPS_HPP
