#pragma once
#include <nlohmann/json.hpp>
#include <vix/json/build.hpp>
#include <vix/json/loads.hpp>
#include <vix/json/dumps.hpp>
#include <vix/json/jpath.hpp>
#include <vix/json/convert.hpp>

/**
 * @file vix/json.hpp
 * @brief Unified header for the Vix.cpp JSON utilities.
 *
 * This file aggregates all the JSON helper modules into a single include:
 * - **Build.hpp** — builders for objects and arrays (`o()`, `a()`, `kv()`).
 * - **Loads.hpp** — parsing from strings and files.
 * - **Dumps.hpp** — stringification and safe file writing.
 * - **JPath.hpp** — navigation and mutation via string paths (`jget`, `jset`).
 * - **Convert.hpp** — type-safe extraction (`get_opt`, `get_or`, `ensure`).
 *
 * The utilities wrap the `nlohmann::json` API with a consistent and expressive
 * functional interface, offering higher safety and conciseness in C++.
 *
 * ### Example
 * @code
 * #include <vix/json.hpp>
 * using namespace Vix::json;
 *
 * int main() {
 *     // --- Build JSON programmatically ---
 *     Json j = o("name", "Softadastra", "features", a("p2p", "blockchain", "ai"));
 *
 *     // --- Access using JPath ---
 *     std::cout << jget(j, "features[1]")->get<std::string>() << std::endl; // "blockchain"
 *
 *     // --- Modify dynamically ---
 *     jset(j, "meta.version", "1.0.0");
 *
 *     // --- Save to disk ---
 *     dump_file("output.json", j);
 *
 *     // --- Reload from disk ---
 *     auto reloaded = load_file("output.json");
 *     std::cout << dumps_pretty(reloaded) << std::endl;
 * }
 * @endcode
 */

namespace Vix::json
{
    /// Alias principal vers `nlohmann::json`, pour compatibilité complète.
    using Json = nlohmann::json;

    /**
     * @brief Create an empty JSON object.
     * @return A new `Json` of type object (`{}`).
     *
     * @code
     * Json j = obj();
     * j["key"] = "value";
     * @endcode
     */
    inline Json obj() { return Json::object(); }

    /**
     * @brief Create an empty JSON array.
     * @return A new `Json` of type array (`[]`).
     *
     * @code
     * Json j = arr();
     * j.push_back(42);
     * j.push_back("hello");
     * @endcode
     */
    inline Json arr() { return Json::array(); }

    /**
     * @brief Import the `"_json"` literal namespace from *nlohmann::json*.
     *
     * Allows direct usage of JSON string literals:
     * @code
     * using namespace Vix::json::literals;
     * Json j = R"({"a": 1, "b": 2})"_json;
     * @endcode
     */
    namespace literals = nlohmann::literals;
}
