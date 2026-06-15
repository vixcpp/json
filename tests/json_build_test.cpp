#include <vix/json/build.hpp>

#include <cassert>
#include <string>
#include <type_traits>

int main()
{
  using namespace vix::json;

  // o() returns OrderedJson
  {
    auto j = o("id", 42, "name", "Gaspard", "active", true);

    static_assert(std::is_same_v<decltype(j), OrderedJson>);

    assert(j.is_object());
    assert(j["id"] == 42);
    assert(j["name"] == "Gaspard");
    assert(j["active"] == true);
  }

  // o() keeps deterministic insertion order
  {
    auto j = o("first", 1, "second", 2, "third", 3);

    const std::string dumped = j.dump();

    assert(dumped.find("first") < dumped.find("second"));
    assert(dumped.find("second") < dumped.find("third"));
  }

  // o() supports nested arrays and objects
  {
    auto j = o(
        "user", o("id", 1, "name", "Ada"),
        "skills", a("C++", "Networking", "Systems"));

    assert(j["user"].is_object());
    assert(j["user"]["id"] == 1);
    assert(j["user"]["name"] == "Ada");

    assert(j["skills"].is_array());
    assert(j["skills"].size() == 3);
    assert(j["skills"][0] == "C++");
    assert(j["skills"][1] == "Networking");
    assert(j["skills"][2] == "Systems");
  }

  // a() creates a JSON array
  {
    Json arr = a(1, 2, 3, "hello", true);

    assert(arr.is_array());
    assert(arr.size() == 5);
    assert(arr[0] == 1);
    assert(arr[1] == 2);
    assert(arr[2] == 3);
    assert(arr[3] == "hello");
    assert(arr[4] == true);
  }

  // a() supports empty arrays
  {
    Json arr = a();

    assert(arr.is_array());
    assert(arr.empty());
  }

  // o() supports empty objects
  {
    auto j = o();

    assert(j.is_object());
    assert(j.empty());
  }

  // kv() creates a JSON object from string_view/json pairs
  {
    Json j = kv({
        {"name", Json("Vix.cpp")},
        {"version", Json("2.6.2")},
        {"debug", Json(true)},
        {"count", Json(3)},
    });

    assert(j.is_object());
    assert(j["name"] == "Vix.cpp");
    assert(j["version"] == "2.6.2");
    assert(j["debug"] == true);
    assert(j["count"] == 3);
  }

  // kv() supports empty objects
  {
    Json j = kv({});

    assert(j.is_object());
    assert(j.empty());
  }

  // Compile-time behavior:
  // This must NOT compile because o() requires key/value pairs:
  //
  // auto bad = o("name", "Vix.cpp", "missing_value");

  return 0;
}
