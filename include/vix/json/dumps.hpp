#ifndef VIX_DUMPS_HPP
#define VIX_DUMPS_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <fstream>

namespace Vix::json
{
    using Json = nlohmann::json;

    inline std::string dumps(const Json &j, int indent = 2, bool ensure_ascii = false)
    {
        if (ensure_ascii)
            return j.dump(indent);
        return j.dump(indent);
    }

    inline void dump_file(const std::string &path, const Json &j, int indent = 2)
    {
        std::ofstream f(path);
        if (!f)
            throw std::runtime_error("Cannot write file: " + path);
        f << j.dump(indent);
    }
}

#endif