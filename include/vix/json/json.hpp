/**
 *
 *  @file json.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/vixcpp/vix
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */

// Backward-compatible guard alias.
#ifndef VIX_MODULE_JSON_HPP
#define VIX_MODULE_JSON_HPP
#endif

#ifndef VIX_JSON_JSON_HPP
#define VIX_JSON_JSON_HPP

#include <nlohmann/json.hpp>

#include <vix/json/build.hpp>
#include <vix/json/loads.hpp>
#include <vix/json/dumps.hpp>
#include <vix/json/jpath.hpp>
#include <vix/json/convert.hpp>

/**
 * @file json.hpp
 * @brief One include for Vix.cpp JSON utilities (nlohmann::json + helpers).
 *
 * @details
 * This header is the recommended entry point when you want to use JSON in Vix.cpp.
 * It re-exports:
 * - builders (`o()`, `a()`, `kv()`)
 * - parsing (`loads()`, `load_file()`)
 * - serialization (`dumps_pretty()`, `dump_file()`)
 * - path navigation (`jget()`, `jset()`)
 * - safe conversions (`get_opt()`, `get_or()`, `ensure()`)
 *
 * For beginners: include this file and you get everything you need.
 *
 * For advanced users: you can include only what you need (`build.hpp`, `jpath.hpp`, etc.)
 * to reduce compile time.
 *
 * ---
 *
 * ## Quick start (beginner)
 * @code
 * #include <vix/json.hpp>
 * using namespace vix::json;
 *
 * int main()
 * {
 *   Json j = o("name", "Softadastra", "features", a("p2p", "blockchain", "ai"));
 *
 *   const Json* second = jget(j, "features[1]");
 *   if (second)
 *     std::cout << second->get<std::string>() << "\n"; // "blockchain"
 *
 *   jset(j, "meta.version", "1.0.0");
 *   dump_file("output.json", j);
 *
 *   Json reloaded = load_file("output.json");
 *   std::cout << dumps_pretty(reloaded) << "\n";
 * }
 * @endcode
 *
 * ---
 *
 * ## Notes for experts
 * - `Json` is an alias of `nlohmann::json` (full compatibility).
 * - `OrderedJson` is an alias of `nlohmann::ordered_json`.
 * - `o()` returns `OrderedJson` to keep deterministic object key order.
 * - `literals` exposes `"_json"` and related literal helpers from nlohmann.
 *
 * @see vix/json/build.hpp
 * @see vix/json/loads.hpp
 * @see vix/json/dumps.hpp
 * @see vix/json/jpath.hpp
 * @see vix/json/convert.hpp
 */

namespace vix::json
{
  /// Main JSON type (fully compatible with nlohmann::json).
  using Json = nlohmann::json;

  /// Ordered JSON type (deterministic object iteration order).
  using OrderedJson = nlohmann::ordered_json;

  /**
   * @brief Create an empty JSON object (`{}`).
   *
   * @return A JSON object.
   *
   * @code
   * Json j = obj();
   * j["key"] = "value";
   * @endcode
   */
  inline Json obj() { return Json::object(); }

  /**
   * @brief Create an empty JSON array (`[]`).
   *
   * @return A JSON array.
   *
   * @code
   * Json j = arr();
   * j.push_back(42);
   * j.push_back("hello");
   * @endcode
   */
  inline Json arr() { return Json::array(); }

  /**
   * @brief JSON string literal helpers from nlohmann.
   *
   * Enables:
   * @code
   * using namespace vix::json::literals;
   * Json j = R"({"a": 1, "b": 2})"_json;
   * @endcode
   */
  namespace literals = nlohmann::literals;

} // namespace vix::json

#endif // VIX_JSON_JSON_HPP
