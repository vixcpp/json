#include <vix/json/dumps.hpp>
#include <vix/json/build.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
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

  vix::json::Json make_medium_json()
  {
    using namespace vix::json;

    return Json{
        {"id", 42},
        {"name", "Vix.cpp"},
        {"module", "json"},
        {"active", true},
        {"score", 9.5},
        {"tags", Json::array({"c++", "runtime", "json", "vix"})},
        {"meta",
         Json{
             {"version", "2.6.2"},
             {"author", "Gaspard"},
             {"city", "Kampala"},
         }},
    };
  }

  vix::json::Json make_large_json()
  {
    using namespace vix::json;

    Json root = Json::object();
    root["name"] = "Vix.cpp JSON benchmark";
    root["active"] = true;
    root["items"] = Json::array();

    for (int i = 0; i < 100; ++i)
    {
      root["items"].push_back(Json{
          {"id", i},
          {"name", std::string("item-") + std::to_string(i)},
          {"enabled", i % 2 == 0},
          {"score", i * 1.25},
          {"tags", Json::array({"json", "benchmark", "vix"})},
      });
    }

    return root;
  }
} // namespace

int main()
{
  using namespace vix::json;

  namespace fs = std::filesystem;

  constexpr int small_iterations = 200000;
  constexpr int medium_iterations = 100000;
  constexpr int large_iterations = 10000;
  constexpr int file_iterations = 1000;

  std::uint64_t checksum = 0;

  Json small = {
      {"id", 1},
      {"name", "Vix.cpp"},
      {"active", true},
  };

  Json medium = make_medium_json();
  Json large = make_large_json();

  bench("dumps() small pretty", small_iterations, [&](int)
        {
          std::string s = dumps(small);

          checksum += static_cast<std::uint64_t>(s.size()); });

  bench("dumps_compact() small", small_iterations, [&](int)
        {
          std::string s = dumps_compact(small);

          checksum += static_cast<std::uint64_t>(s.size()); });

  bench("dumps() medium pretty", medium_iterations, [&](int)
        {
          std::string s = dumps(medium);

          checksum += static_cast<std::uint64_t>(s.size()); });

  bench("dumps_compact() medium", medium_iterations, [&](int)
        {
          std::string s = dumps_compact(medium);

          checksum += static_cast<std::uint64_t>(s.size()); });

  bench("dumps_pretty() medium", medium_iterations, [&](int)
        {
          std::string s = dumps_pretty(medium);

          checksum += static_cast<std::uint64_t>(s.size()); });

  bench("dumps() large pretty", large_iterations, [&](int)
        {
          std::string s = dumps(large);

          checksum += static_cast<std::uint64_t>(s.size()); });

  bench("dumps_compact() large", large_iterations, [&](int)
        {
          std::string s = dumps_compact(large);

          checksum += static_cast<std::uint64_t>(s.size()); });

  bench("dumps_compact() ensure_ascii=false", medium_iterations, [&](int)
        {
          Json j = {
              {"word", "café"},
              {"city", "Kampala"},
              {"runtime", "Vix.cpp"},
          };

          std::string s = dumps_compact(j, false);

          checksum += static_cast<std::uint64_t>(s.size()); });

  bench("dumps_compact() ensure_ascii=true", medium_iterations, [&](int)
        {
          Json j = {
              {"word", "café"},
              {"city", "Kampala"},
              {"runtime", "Vix.cpp"},
          };

          std::string s = dumps_compact(j, true);

          checksum += static_cast<std::uint64_t>(s.size()); });

  bench("manual nlohmann dump compact baseline", medium_iterations, [&](int)
        {
          std::string s = medium.dump(-1, ' ', false);

          checksum += static_cast<std::uint64_t>(s.size()); });

  bench("manual nlohmann dump pretty baseline", medium_iterations, [&](int)
        {
          std::string s = medium.dump(2, ' ', false);

          checksum += static_cast<std::uint64_t>(s.size()); });

  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_dumps_bench";
    const fs::path path = dir / "out.json";

    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);

    bench("dump_file() medium", file_iterations, [&](int i)
          {
            Json j = medium;
            j["iteration"] = i;

            dump_file(path, j);

            checksum += static_cast<std::uint64_t>(fs::file_size(path)); });

    bench("dump_file() large", file_iterations, [&](int i)
          {
            Json j = large;
            j["iteration"] = i;

            dump_file(path, j);

            checksum += static_cast<std::uint64_t>(fs::file_size(path)); });

    fs::remove_all(dir, ec);
  }

  std::cout << "checksum: " << checksum << "\n";

  return 0;
}
