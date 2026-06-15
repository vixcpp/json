#include <vix/json/jpath.hpp>
#include <vix/json/build.hpp>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>

namespace
{
  template <class Fn>
  void bench(const std::string &name, int iterations, Fn fn)
  {
    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < iterations; ++i)
      fn(i);

    auto end = std::chrono::steady_clock::now();

    const auto us =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << name << "\n";
    std::cout << "  iterations: " << iterations << "\n";
    std::cout << "  total:      " << us << " us\n";
    std::cout << "  avg:        " << static_cast<double>(us) / iterations << " us/op\n\n";
  }

  vix::json::Json make_sample_json()
  {
    using namespace vix::json;

    Json root = Json::object();

    root["user"] = Json{
        {"id", 42},
        {"profile",
         Json{
             {"name", "Gaspard"},
             {"city", "Kampala"},
             {"active", true},
         }},
        {"roles", Json::array({"admin", "developer", "maintainer"})},
    };

    root["settings"] = Json{
        {"theme", "dark"},
        {"language", "fr"},
        {"limits",
         Json{
             {"max_connections", 100},
             {"timeout_ms", 3000},
         }},
    };

    root["users"] = Json::array();

    for (int i = 0; i < 100; ++i)
    {
      root["users"].push_back(Json{
          {"id", i},
          {"email", std::string("user") + std::to_string(i) + "@example.com"},
          {"active", i % 2 == 0},
          {"score", i * 1.5},
      });
    }

    root["complex.key"] = Json{
        {"value", 123},
    };

    root["a b c"] = Json::array({"first", "second", "third"});

    return root;
  }
} // namespace

int main()
{
  using namespace vix::json;

  constexpr int fast_iterations = 1000000;
  constexpr int medium_iterations = 500000;
  constexpr int write_iterations = 100000;
  constexpr int tokenize_iterations = 500000;

  std::uint64_t checksum = 0;

  Json root = make_sample_json();
  const Json &const_root = root;

  bench("tokenize_path() dot path", tokenize_iterations, [&](int)
        {
          auto toks = tokenize_path("user.profile.name");

          checksum += static_cast<std::uint64_t>(toks.size()); });

  bench("tokenize_path() array path", tokenize_iterations, [&](int)
        {
          auto toks = tokenize_path("users[25].email");

          checksum += static_cast<std::uint64_t>(toks.size());
          checksum += static_cast<std::uint64_t>(toks[1].index); });

  bench("tokenize_path() quoted key path", tokenize_iterations, [&](int)
        {
          auto toks = tokenize_path(R"(["complex.key"].value)");

          checksum += static_cast<std::uint64_t>(toks.size());
          checksum += static_cast<std::uint64_t>(toks[0].key.size()); });

  bench("jget const simple dot path", fast_iterations, [&](int)
        {
          const Json *value = jget(const_root, "user.profile.name");

          if (value)
            checksum += static_cast<std::uint64_t>(value->get<std::string>().size()); });

  bench("jget const nested number path", fast_iterations, [&](int)
        {
          const Json *value = jget(const_root, "settings.limits.timeout_ms");

          if (value)
            checksum += static_cast<std::uint64_t>(value->get<int>()); });

  bench("jget const array path", fast_iterations, [&](int)
        {
          const Json *value = jget(const_root, "users[25].email");

          if (value)
            checksum += static_cast<std::uint64_t>(value->get<std::string>().size()); });

  bench("jget const quoted key path", medium_iterations, [&](int)
        {
          const Json *value = jget(const_root, R"(["complex.key"].value)");

          if (value)
            checksum += static_cast<std::uint64_t>(value->get<int>()); });

  bench("jget const quoted key with spaces path", medium_iterations, [&](int)
        {
          const Json *value = jget(const_root, R"(["a b c"][1])");

          if (value)
            checksum += static_cast<std::uint64_t>(value->get<std::string>().size()); });

  bench("jget const missing key", fast_iterations, [&](int)
        {
          const Json *value = jget(const_root, "user.profile.missing");

          checksum += value == nullptr ? 1 : 0; });

  bench("jget const out-of-range index", fast_iterations, [&](int)
        {
          const Json *value = jget(const_root, "users[999].email");

          checksum += value == nullptr ? 1 : 0; });

  bench("manual object access baseline", fast_iterations, [&](int)
        {
          const Json &value = const_root["user"]["profile"]["name"];

          checksum += static_cast<std::uint64_t>(value.get<std::string>().size()); });

  bench("manual array access baseline", fast_iterations, [&](int)
        {
          const Json &value = const_root["users"][25]["email"];

          checksum += static_cast<std::uint64_t>(value.get<std::string>().size()); });

  bench("jset existing simple path", write_iterations, [&](int i)
        {
          Json j = root;

          bool ok = jset(j, "user.profile.name", std::string("name-") + std::to_string(i));

          checksum += ok ? 1 : 0;
          checksum += static_cast<std::uint64_t>(j["user"]["profile"]["name"].get<std::string>().size()); });

  bench("jset existing array path", write_iterations, [&](int i)
        {
          Json j = root;

          bool ok = jset(j, "users[10].score", i);

          checksum += ok ? 1 : 0;
          checksum += static_cast<std::uint64_t>(j["users"][10]["score"].get<int>()); });

  bench("jset create nested object path", write_iterations, [&](int i)
        {
          Json j = Json::object();

          bool ok = jset(j, "app.config.runtime.name", std::string("vix-") + std::to_string(i));

          checksum += ok ? 1 : 0;
          checksum += static_cast<std::uint64_t>(j["app"]["config"]["runtime"]["name"].get<std::string>().size()); });

  bench("jset create array path", write_iterations, [&](int i)
        {
          Json j = Json::object();

          bool ok = jset(j, "items[5].id", i);

          checksum += ok ? 1 : 0;
          checksum += static_cast<std::uint64_t>(j["items"].size());
          checksum += static_cast<std::uint64_t>(j["items"][5]["id"].get<int>()); });

  bench("jset quoted key path", write_iterations, [&](int i)
        {
          Json j = Json::object();

          bool ok = jset(j, R"(["complex.key"].value)", i);

          checksum += ok ? 1 : 0;
          checksum += static_cast<std::uint64_t>(j["complex.key"]["value"].get<int>()); });

  bench("mutable jget create nested object path", write_iterations, [&](int i)
        {
          Json j = Json::object();

          Json *value = jget(j, "runtime.config.version");

          *value = i;

          checksum += static_cast<std::uint64_t>(j["runtime"]["config"]["version"].get<int>()); });

  bench("mutable jget create array path", write_iterations, [&](int i)
        {
          Json j = Json::object();

          Json *value = jget(j, "items[10].value");

          *value = i;

          checksum += static_cast<std::uint64_t>(j["items"].size());
          checksum += static_cast<std::uint64_t>(j["items"][10]["value"].get<int>()); });

  bench("jset invalid path", write_iterations, [&](int i)
        {
          Json j = Json::object();

          bool ok = jset(j, "items[bad].value", i);

          checksum += ok ? 0 : 1; });

  std::cout << "checksum: " << checksum << "\n";

  return 0;
}
