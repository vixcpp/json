#include <vix/json/convert.hpp>
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

  constexpr int fast_iterations = 1000000;
  constexpr int medium_iterations = 500000;
  constexpr int slow_iterations = 100000;

  std::uint64_t checksum = 0;

  Json obj = {
      {"id", 42},
      {"name", "Vix.cpp"},
      {"active", true},
      {"score", 9.5},
      {"port", 8080},
  };

  Json arr = Json::array({
      10,
      20,
      30,
      "hello",
      true,
  });

  bench("ptr(obj, key) existing", fast_iterations, [&](int)
        {
          const Json *p = ptr(obj, "id");

          if (p)
            checksum += static_cast<std::uint64_t>(p->get<int>()); });

  bench("ptr(obj, key) missing", fast_iterations, [&](int)
        {
          const Json *p = ptr(obj, "missing");

          checksum += p == nullptr ? 1 : 0; });

  bench("ptr(array, index) existing", fast_iterations, [&](int)
        {
          const Json *p = ptr(arr, 1);

          if (p)
            checksum += static_cast<std::uint64_t>(p->get<int>()); });

  bench("ptr(array, index) out of range", fast_iterations, [&](int)
        {
          const Json *p = ptr(arr, 100);

          checksum += p == nullptr ? 1 : 0; });

  bench("get_opt<int>(json)", fast_iterations, [&](int)
        {
          auto value = get_opt<int>(obj["id"]);

          if (value)
            checksum += static_cast<std::uint64_t>(*value); });

  bench("get_opt<string>(json)", medium_iterations, [&](int)
        {
          auto value = get_opt<std::string>(obj["name"]);

          if (value)
            checksum += static_cast<std::uint64_t>(value->size()); });

  bench("get_opt<int>(obj, key)", fast_iterations, [&](int)
        {
          auto value = get_opt<int>(obj, "port");

          if (value)
            checksum += static_cast<std::uint64_t>(*value); });

  bench("get_opt<int>(obj, missing)", fast_iterations, [&](int)
        {
          auto value = get_opt<int>(obj, "missing");

          checksum += value ? 0 : 1; });

  bench("get_opt<int>(array, index)", fast_iterations, [&](int)
        {
          auto value = get_opt<int>(arr, 2);

          if (value)
            checksum += static_cast<std::uint64_t>(*value); });

  bench("get_opt<int>(array, missing)", fast_iterations, [&](int)
        {
          auto value = get_opt<int>(arr, 100);

          checksum += value ? 0 : 1; });

  bench("get_or<int>(json, default) valid", fast_iterations, [&](int)
        {
          int value = get_or<int>(obj["id"], -1);

          checksum += static_cast<std::uint64_t>(value); });

  bench("get_or<int>(json, default) invalid", fast_iterations, [&](int)
        {
          int value = get_or<int>(obj["name"], -1);

          checksum += static_cast<std::uint64_t>(value == -1 ? 1 : 0); });

  bench("get_or<int>(obj, key, default)", fast_iterations, [&](int)
        {
          int value = get_or<int>(obj, "port", -1);

          checksum += static_cast<std::uint64_t>(value); });

  bench("get_or<int>(array, index, default)", fast_iterations, [&](int)
        {
          int value = get_or<int>(arr, 0, -1);

          checksum += static_cast<std::uint64_t>(value); });

  bench("ensure<int>(json)", fast_iterations, [&](int)
        {
          int value = ensure<int>(obj["id"]);

          checksum += static_cast<std::uint64_t>(value); });

  bench("ensure<int>(obj, key)", fast_iterations, [&](int)
        {
          int value = ensure<int>(obj, "port");

          checksum += static_cast<std::uint64_t>(value); });

  bench("simple_to_json(token primitives)", medium_iterations, [&](int i)
        {
          token id = i;
          token name = "Vix.cpp";
          token active = true;

          Json jid = simple_to_json(id);
          Json jname = simple_to_json(name);
          Json jactive = simple_to_json(active);

          checksum += static_cast<std::uint64_t>(jid.get<int>());
          checksum += static_cast<std::uint64_t>(jname.get<std::string>().size());
          checksum += jactive.get<bool>() ? 1 : 0; });

  bench("simple_to_json(array_t)", slow_iterations, [&](int i)
        {
          array_t values = simple_array({
              i,
              i + 1,
              i + 2,
              "json",
              true,
          });

          Json j = simple_to_json(values);

          checksum += static_cast<std::uint64_t>(j.size());
          checksum += static_cast<std::uint64_t>(j[0].get<int>()); });

  bench("simple_to_json(kvs)", slow_iterations, [&](int i)
        {
          kvs user = simple_obj({
              "id", i,
              "name", "Ada",
              "active", true,
              "skills", simple_array({"C++", "JSON", "Vix.cpp"}),
          });

          Json j = simple_to_json(user);

          checksum += static_cast<std::uint64_t>(j.size());
          checksum += static_cast<std::uint64_t>(j["id"].get<int>());
          checksum += static_cast<std::uint64_t>(j["skills"].size()); });

  bench("manual object access baseline", fast_iterations, [&](int)
        {
          if (obj.contains("id") && obj["id"].is_number_integer())
            checksum += static_cast<std::uint64_t>(obj["id"].get<int>()); });

  bench("manual array access baseline", fast_iterations, [&](int)
        {
          if (arr.is_array() && arr.size() > 1 && arr[1].is_number_integer())
            checksum += static_cast<std::uint64_t>(arr[1].get<int>()); });

  std::cout << "checksum: " << checksum << "\n";

  return 0;
}
