#include <vix/json/loads.hpp>
#include <vix/json/dumps.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
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

  std::string make_small_json_text()
  {
    return R"({"id":42,"name":"Vix.cpp","active":true})";
  }

  std::string make_medium_json_text()
  {
    return R"({
      "id": 42,
      "name": "Vix.cpp",
      "module": "json",
      "active": true,
      "score": 9.5,
      "tags": ["c++", "runtime", "json", "vix"],
      "meta": {
        "version": "2.6.2",
        "author": "Gaspard",
        "city": "Kampala"
      }
    })";
  }

  std::string make_large_json_text()
  {
    vix::json::Json root = vix::json::Json::object();

    root["name"] = "Vix.cpp JSON loads benchmark";
    root["active"] = true;
    root["items"] = vix::json::Json::array();

    for (int i = 0; i < 100; ++i)
    {
      root["items"].push_back(vix::json::Json{
          {"id", i},
          {"name", std::string("item-") + std::to_string(i)},
          {"enabled", i % 2 == 0},
          {"score", i * 1.25},
          {"tags", vix::json::Json::array({"json", "benchmark", "vix"})},
          {"meta",
           vix::json::Json{
               {"created_by", "benchmark"},
               {"index", i},
               {"kind", "sample"},
           }},
      });
    }

    return root.dump();
  }

  void write_text_file(const std::filesystem::path &path, const std::string &text)
  {
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    ofs << text;
  }
} // namespace

int main()
{
  using namespace vix::json;

  namespace fs = std::filesystem;

  constexpr int small_iterations = 300000;
  constexpr int medium_iterations = 100000;
  constexpr int large_iterations = 10000;
  constexpr int file_iterations = 3000;
  constexpr int invalid_iterations = 100000;

  std::uint64_t checksum = 0;

  const std::string small_text = make_small_json_text();
  const std::string medium_text = make_medium_json_text();
  const std::string large_text = make_large_json_text();

  bench("loads() small object", small_iterations, [&](int)
        {
          Json j = loads(small_text);

          checksum += static_cast<std::uint64_t>(j.size());
          checksum += static_cast<std::uint64_t>(j["id"].get<int>()); });

  bench("try_loads() small object", small_iterations, [&](int)
        {
          auto maybe = try_loads(small_text);

          if (maybe)
          {
            checksum += static_cast<std::uint64_t>(maybe->size());
            checksum += static_cast<std::uint64_t>((*maybe)["id"].get<int>());
          } });

  bench("loads() medium object", medium_iterations, [&](int)
        {
          Json j = loads(medium_text);

          checksum += static_cast<std::uint64_t>(j.size());
          checksum += static_cast<std::uint64_t>(j["id"].get<int>());
          checksum += static_cast<std::uint64_t>(j["tags"].size()); });

  bench("try_loads() medium object", medium_iterations, [&](int)
        {
          auto maybe = try_loads(medium_text);

          if (maybe)
          {
            checksum += static_cast<std::uint64_t>(maybe->size());
            checksum += static_cast<std::uint64_t>((*maybe)["id"].get<int>());
            checksum += static_cast<std::uint64_t>((*maybe)["tags"].size());
          } });

  bench("loads() large object", large_iterations, [&](int)
        {
          Json j = loads(large_text);

          checksum += static_cast<std::uint64_t>(j.size());
          checksum += static_cast<std::uint64_t>(j["items"].size());
          checksum += static_cast<std::uint64_t>(j["items"][50]["id"].get<int>()); });

  bench("try_loads() large object", large_iterations, [&](int)
        {
          auto maybe = try_loads(large_text);

          if (maybe)
          {
            checksum += static_cast<std::uint64_t>(maybe->size());
            checksum += static_cast<std::uint64_t>((*maybe)["items"].size());
            checksum += static_cast<std::uint64_t>((*maybe)["items"][50]["id"].get<int>());
          } });

  bench("manual nlohmann parse small baseline", small_iterations, [&](int)
        {
          Json j = Json::parse(small_text);

          checksum += static_cast<std::uint64_t>(j.size());
          checksum += static_cast<std::uint64_t>(j["id"].get<int>()); });

  bench("manual nlohmann parse medium baseline", medium_iterations, [&](int)
        {
          Json j = Json::parse(medium_text);

          checksum += static_cast<std::uint64_t>(j.size());
          checksum += static_cast<std::uint64_t>(j["id"].get<int>()); });

  bench("try_loads() invalid json", invalid_iterations, [&](int)
        {
          auto maybe = try_loads("{bad json");

          checksum += maybe ? 0 : 1; });

  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_loads_bench";
    const fs::path small_path = dir / "small.json";
    const fs::path medium_path = dir / "medium.json";
    const fs::path large_path = dir / "large.json";
    const fs::path invalid_path = dir / "invalid.json";

    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);

    write_text_file(small_path, small_text);
    write_text_file(medium_path, medium_text);
    write_text_file(large_path, large_text);
    write_text_file(invalid_path, "{bad json");

    bench("load_file() small file", file_iterations, [&](int)
          {
            Json j = load_file(small_path);

            checksum += static_cast<std::uint64_t>(j.size());
            checksum += static_cast<std::uint64_t>(j["id"].get<int>()); });

    bench("try_load_file() small file", file_iterations, [&](int)
          {
            auto maybe = try_load_file(small_path);

            if (maybe)
            {
              checksum += static_cast<std::uint64_t>(maybe->size());
              checksum += static_cast<std::uint64_t>((*maybe)["id"].get<int>());
            } });

    bench("load_file() medium file", file_iterations, [&](int)
          {
            Json j = load_file(medium_path);

            checksum += static_cast<std::uint64_t>(j.size());
            checksum += static_cast<std::uint64_t>(j["id"].get<int>());
            checksum += static_cast<std::uint64_t>(j["tags"].size()); });

    bench("try_load_file() medium file", file_iterations, [&](int)
          {
            auto maybe = try_load_file(medium_path);

            if (maybe)
            {
              checksum += static_cast<std::uint64_t>(maybe->size());
              checksum += static_cast<std::uint64_t>((*maybe)["id"].get<int>());
              checksum += static_cast<std::uint64_t>((*maybe)["tags"].size());
            } });

    bench("load_file() large file", file_iterations, [&](int)
          {
            Json j = load_file(large_path);

            checksum += static_cast<std::uint64_t>(j.size());
            checksum += static_cast<std::uint64_t>(j["items"].size());
            checksum += static_cast<std::uint64_t>(j["items"][50]["id"].get<int>()); });

    bench("try_load_file() large file", file_iterations, [&](int)
          {
            auto maybe = try_load_file(large_path);

            if (maybe)
            {
              checksum += static_cast<std::uint64_t>(maybe->size());
              checksum += static_cast<std::uint64_t>((*maybe)["items"].size());
              checksum += static_cast<std::uint64_t>((*maybe)["items"][50]["id"].get<int>());
            } });

    bench("try_load_file() invalid file", file_iterations, [&](int)
          {
            auto maybe = try_load_file(invalid_path);

            checksum += maybe ? 0 : 1; });

    fs::remove_all(dir, ec);
  }

  std::cout << "checksum: " << checksum << "\n";

  return 0;
}
