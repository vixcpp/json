/**
 * @file fwd.hpp
 * @brief Lightweight declarations for Vix JSON public signatures.
 */
#ifndef VIX_JSON_FWD_HPP
#define VIX_JSON_FWD_HPP

#include <nlohmann/json_fwd.hpp>

namespace vix::json
{
  using Json = nlohmann::json;
  using OrderedJson = nlohmann::ordered_json;
}

#endif // VIX_JSON_FWD_HPP
