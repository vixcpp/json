#ifndef VIX_JSON_HPP
#define VIX_JSON_HPP

#include <nlohmann/json.hpp>
#include "build.hpp"
#include "loads.hpp"
#include "dumps.hpp"
#include "jpath.hpp"
#include "convert.hpp"

namespace Vix::json
{
    using Json = nlohmann::json;

    inline auto obj() -> Json { return Json::object(); }
    inline auto arr() -> Json { return Json::array(); }
}

#endif