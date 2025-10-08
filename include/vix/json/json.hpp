#pragma once
#include <nlohmann/json.hpp>
#include <vix/json/build.hpp>
#include <vix/json/loads.hpp>
#include <vix/json/dumps.hpp>
#include <vix/json/jpath.hpp>
#include <vix/json/convert.hpp>

namespace Vix::json
{
    using Json = nlohmann::json;
    inline Json obj() { return Json::object(); }
    inline Json arr() { return Json::array(); }
    namespace literals = nlohmann::literals;
}
