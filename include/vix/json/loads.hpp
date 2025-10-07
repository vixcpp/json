#ifndef VIX_LOADS_HPP
#define VIX_LOADS_HPP

#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

namespace Vix::json
{
    using Json = nlohmann::json;

    inline Json loads(const std::string &s)
    {
        return Json::parse(s, /*callback*/ nullptr, /*allow_exceptions*/ true);
    }

    inline Json load_file(const std::string &path)
    {
        std::ifstream f(path);
        if (!f)
            throw std::runtime_error("Cannot open file: " + path);
        Json j;
        f >> j;
        return j;
    }
}

#endif
