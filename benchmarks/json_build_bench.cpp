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
} // namespace

int main()
{
  using namespace vix::json;

  constexpr int small_iterations = 200000;
  constexpr int medium_iterations = 100000;
  constexpr int large_iterations = 20000;

  std::uint64_t checksum = 0;

  bench("json::o small object", small_iterations, [&](int i)
        {
          auto j = o(
              "id", i,
              "name", "Vix.cpp",
              "active", true);

          checksum += static_cast<std::uint64_t>(j.size());
          checksum += static_cast<std::uint64_t>(j["id"].get<int>()); });

  bench("json::o medium object", medium_iterations, [&](int i)
        {
          auto j = o(
              "id", i,
              "name", "Gaspard",
              "runtime", "vix",
              "module", "json",
              "active", true,
              "score", 9.5,
              "country", "UG",
              "city", "Kampala");

          checksum += static_cast<std::uint64_t>(j.size());
          checksum += static_cast<std::uint64_t>(j["id"].get<int>()); });

  bench("json::a small array", small_iterations, [&](int i)
        {
          Json arr = a(i, i + 1, i + 2, "json", true);

          checksum += static_cast<std::uint64_t>(arr.size());
          checksum += static_cast<std::uint64_t>(arr[0].get<int>()); });

  bench("json::a medium array", medium_iterations, [&](int i)
        {
          Json arr = a(
              i,
              i + 1,
              i + 2,
              i + 3,
              i + 4,
              "vix",
              "json",
              "benchmark",
              true,
              false,
              nullptr);

          checksum += static_cast<std::uint64_t>(arr.size());
          checksum += static_cast<std::uint64_t>(arr[0].get<int>()); });

  bench("json::o nested object and array", medium_iterations, [&](int i)
        {
          auto j = o(
              "user", o(
                          "id", i,
                          "name", "Ada",
                          "active", true),
              "skills", a("C++", "JSON", "Vix.cpp"),
              "meta", o(
                          "module", "json",
                          "bench", true,
                          "version", 1));

          checksum += static_cast<std::uint64_t>(j.size());
          checksum += static_cast<std::uint64_t>(j["user"]["id"].get<int>());
          checksum += static_cast<std::uint64_t>(j["skills"].size()); });

  bench("json::kv object", medium_iterations, [&](int i)
        {
          Json j = kv({
              {"id", Json(i)},
              {"name", Json("Vix.cpp")},
              {"module", Json("json")},
              {"active", Json(true)},
              {"score", Json(9.5)},
          });

          checksum += static_cast<std::uint64_t>(j.size());
          checksum += static_cast<std::uint64_t>(j["id"].get<int>()); });

  bench("manual nlohmann object baseline", small_iterations, [&](int i)
        {
          Json j = Json::object();

          j["id"] = i;
          j["name"] = "Vix.cpp";
          j["active"] = true;

          checksum += static_cast<std::uint64_t>(j.size());
          checksum += static_cast<std::uint64_t>(j["id"].get<int>()); });

  bench("manual nlohmann array baseline", small_iterations, [&](int i)
        {
          Json arr = Json::array();

          arr.push_back(i);
          arr.push_back(i + 1);
          arr.push_back(i + 2);
          arr.push_back("json");
          arr.push_back(true);

          checksum += static_cast<std::uint64_t>(arr.size());
          checksum += static_cast<std::uint64_t>(arr[0].get<int>()); });

  bench("large generated array", large_iterations, [&](int i)
        {
          Json arr = Json::array();

          for (int n = 0; n < 100; ++n)
            arr.push_back(i + n);

          checksum += static_cast<std::uint64_t>(arr.size());
          checksum += static_cast<std::uint64_t>(arr[0].get<int>());
          checksum += static_cast<std::uint64_t>(arr[99].get<int>()); });

  std::cout << "checksum: " << checksum << "\n";

  return 0;
}
