#include <vix/json/Simple.hpp>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

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

  bench("token construct int", fast_iterations, [&](int i)
        {
          token t = i;

          checksum += static_cast<std::uint64_t>(t.as_i64_or(0)); });

  bench("token construct string", fast_iterations, [&](int i)
        {
          token t = std::string("vix-") + std::to_string(i);

          checksum += static_cast<std::uint64_t>(t.as_string_or("").size()); });

  bench("token setters", fast_iterations, [&](int i)
        {
          token t;

          t.set_int(i);
          checksum += static_cast<std::uint64_t>(t.as_i64_or(0));

          t.set_bool((i % 2) == 0);
          checksum += t.as_bool_or(false) ? 1 : 0;

          t.set_f64(static_cast<double>(i) * 1.5);
          checksum += static_cast<std::uint64_t>(t.as_f64_or(0.0));

          t.set_string("Vix.cpp");
          checksum += static_cast<std::uint64_t>(t.as_string_or("").size()); });

  bench("array_t push_int", fast_iterations, [&](int i)
        {
          array_t arr;

          arr.push_int(i);
          arr.push_int(i + 1);
          arr.push_int(i + 2);
          arr.push_int(i + 3);

          checksum += static_cast<std::uint64_t>(arr.size());
          checksum += static_cast<std::uint64_t>(arr[0].as_i64_or(0)); });

  bench("array_t push mixed values", medium_iterations, [&](int i)
        {
          array_t arr;

          arr.push_int(i);
          arr.push_string("Vix.cpp");
          arr.push_bool(true);
          arr.push_f64(9.5);
          arr.push_null();

          checksum += static_cast<std::uint64_t>(arr.size());
          checksum += static_cast<std::uint64_t>(arr[0].as_i64_or(0));
          checksum += static_cast<std::uint64_t>(arr[1].as_string_or("").size()); });

  bench("array_t ensure grows", medium_iterations, [&](int i)
        {
          array_t arr;

          token &value = arr.ensure(10);
          value = i;

          checksum += static_cast<std::uint64_t>(arr.size());
          checksum += static_cast<std::uint64_t>(arr[10].as_i64_or(0)); });

  bench("array_t erase_at middle", medium_iterations, [&](int i)
        {
          array_t arr = simple_array({1, 2, 3, 4, 5, 6, 7, 8});

          bool ok = arr.erase_at(static_cast<std::size_t>(i % 8));

          checksum += ok ? 1 : 0;
          checksum += static_cast<std::uint64_t>(arr.size()); });

  bench("array_t build 100 integers", slow_iterations, [&](int i)
        {
          array_t arr;
          arr.reserve(100);

          for (int n = 0; n < 100; ++n)
            arr.push_int(i + n);

          checksum += static_cast<std::uint64_t>(arr.size());
          checksum += static_cast<std::uint64_t>(arr[0].as_i64_or(0));
          checksum += static_cast<std::uint64_t>(arr[99].as_i64_or(0)); });

  bench("kvs set new keys", medium_iterations, [&](int i)
        {
          kvs obj;

          obj.set_int("id", i);
          obj.set_string("name", "Vix.cpp");
          obj.set_bool("active", true);
          obj.set_f64("score", 9.5);

          checksum += static_cast<std::uint64_t>(obj.size_pairs());
          checksum += static_cast<std::uint64_t>(obj.get_i64_or("id"));
          checksum += static_cast<std::uint64_t>(obj.get_string_or("name").size()); });

  bench("kvs replace existing key", medium_iterations, [&](int i)
        {
          kvs obj;

          obj.set_int("count", 1);
          obj.set_int("count", i);

          checksum += static_cast<std::uint64_t>(obj.size_pairs());
          checksum += static_cast<std::uint64_t>(obj.get_i64_or("count")); });

  bench("kvs operator[] existing and missing", medium_iterations, [&](int i)
        {
          kvs obj;

          obj["id"] = i;
          obj["name"] = "Vix.cpp";

          checksum += static_cast<std::uint64_t>(obj["id"].as_i64_or(0));
          checksum += static_cast<std::uint64_t>(obj["name"].as_string_or("").size());
          checksum += static_cast<std::uint64_t>(obj.size_pairs()); });

  bench("kvs contains existing key", fast_iterations, [&](int)
        {
          kvs obj = simple_obj({
              "id", 42,
              "name", "Vix.cpp",
              "active", true,
              "port", 8080,
          });

          checksum += obj.contains("name") ? 1 : 0; });

  bench("kvs contains missing key", fast_iterations, [&](int)
        {
          kvs obj = simple_obj({
              "id", 42,
              "name", "Vix.cpp",
              "active", true,
              "port", 8080,
          });

          checksum += obj.contains("missing") ? 0 : 1; });

  bench("kvs typed getters", fast_iterations, [&](int)
        {
          kvs obj = simple_obj({
              "id", 42,
              "name", "Vix.cpp",
              "active", true,
              "score", 9.5,
          });

          checksum += static_cast<std::uint64_t>(obj.get_i64_or("id"));
          checksum += static_cast<std::uint64_t>(obj.get_string_or("name").size());
          checksum += obj.get_bool_or("active") ? 1 : 0;
          checksum += static_cast<std::uint64_t>(obj.get_f64_or("score")); });

  bench("kvs erase existing key", medium_iterations, [&](int)
        {
          kvs obj = simple_obj({
              "a", 1,
              "b", 2,
              "c", 3,
              "d", 4,
          });

          bool ok = obj.erase("c");

          checksum += ok ? 1 : 0;
          checksum += static_cast<std::uint64_t>(obj.size_pairs()); });

  bench("kvs erase_if", medium_iterations, [&](int)
        {
          kvs obj = simple_obj({
              "keep_a", 1,
              "remove_a", 2,
              "keep_b", 3,
              "remove_b", 4,
          });

          std::size_t removed = obj.erase_if([](std::string_view key, const token &)
                                             { return key.find("remove_") == 0; });

          checksum += static_cast<std::uint64_t>(removed);
          checksum += static_cast<std::uint64_t>(obj.size_pairs()); });

  bench("kvs for_each_pair", fast_iterations, [&](int)
        {
          kvs obj = simple_obj({
              "a", 1,
              "b", 2,
              "c", 3,
              "d", 4,
          });

          std::uint64_t local = 0;

          obj.for_each_pair([&](std::string_view, const token &value)
                            { local += static_cast<std::uint64_t>(value.as_i64_or(0)); });

          checksum += local; });

  bench("kvs keys()", medium_iterations, [&](int)
        {
          kvs obj = simple_obj({
              "first", 1,
              "second", 2,
              "third", 3,
              "fourth", 4,
          });

          std::vector<std::string> keys = obj.keys();

          checksum += static_cast<std::uint64_t>(keys.size());
          checksum += static_cast<std::uint64_t>(keys[0].size()); });

  bench("kvs merge_from overwrite=true", medium_iterations, [&](int)
        {
          kvs base = simple_obj({
              "name", "old",
              "active", true,
          });

          kvs other = simple_obj({
              "name", "new",
              "age", 30,
              "city", "Kampala",
          });

          std::size_t changes = base.merge_from(other, true);

          checksum += static_cast<std::uint64_t>(changes);
          checksum += static_cast<std::uint64_t>(base.size_pairs());
          checksum += static_cast<std::uint64_t>(base.get_string_or("name").size()); });

  bench("kvs merge_from overwrite=false", medium_iterations, [&](int)
        {
          kvs base = simple_obj({
              "name", "old",
              "active", true,
          });

          kvs other = simple_obj({
              "name", "new",
              "age", 30,
              "city", "Kampala",
          });

          std::size_t changes = base.merge_from(other, false);

          checksum += static_cast<std::uint64_t>(changes);
          checksum += static_cast<std::uint64_t>(base.size_pairs());
          checksum += static_cast<std::uint64_t>(base.get_string_or("name").size()); });

  bench("token ensure_array", medium_iterations, [&](int i)
        {
          token t = "not array";

          array_t &arr = t.ensure_array();
          arr.push_int(i);

          checksum += t.is_array() ? 1 : 0;
          checksum += static_cast<std::uint64_t>(arr.size());
          checksum += static_cast<std::uint64_t>(arr[0].as_i64_or(0)); });

  bench("token ensure_object", medium_iterations, [&](int i)
        {
          token t = "not object";

          kvs &obj = t.ensure_object();
          obj.set_int("id", i);

          checksum += t.is_object() ? 1 : 0;
          checksum += static_cast<std::uint64_t>(obj.size_pairs());
          checksum += static_cast<std::uint64_t>(obj.get_i64_or("id")); });

  bench("nested Simple object build", slow_iterations, [&](int i)
        {
          kvs root;

          kvs &user = root.ensure_object("user");
          user.set_int("id", i);
          user.set_string("name", "Ada");

          array_t &skills = user.ensure_array("skills");
          skills.push_string("C++");
          skills.push_string("JSON");
          skills.push_string("Vix.cpp");

          checksum += static_cast<std::uint64_t>(root.size_pairs());
          checksum += static_cast<std::uint64_t>(user.size_pairs());
          checksum += static_cast<std::uint64_t>(skills.size());
          checksum += static_cast<std::uint64_t>(user.get_i64_or("id")); });

  bench("manual vector<token> array baseline", slow_iterations, [&](int i)
        {
          std::vector<token> values;
          values.reserve(100);

          for (int n = 0; n < 100; ++n)
            values.emplace_back(i + n);

          checksum += static_cast<std::uint64_t>(values.size());
          checksum += static_cast<std::uint64_t>(values[0].as_i64_or(0));
          checksum += static_cast<std::uint64_t>(values[99].as_i64_or(0)); });

  std::cout << "checksum: " << checksum << "\n";

  return 0;
}
