#include <vix/json/convert.hpp>
#include <vix/json/build.hpp>

#include <cassert>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

int main()
{
  using namespace vix::json;

  // ptr(obj, key) returns pointer when key exists
  {
    Json j = {
        {"id", 42},
        {"name", "Gaspard"},
        {"active", true},
    };

    const Json *id = ptr(j, "id");
    const Json *name = ptr(j, "name");
    const Json *missing = ptr(j, "missing");

    assert(id != nullptr);
    assert(name != nullptr);
    assert(missing == nullptr);

    assert(*id == 42);
    assert(*name == "Gaspard");
  }

  // ptr(obj, key) returns nullptr when JSON is not an object
  {
    Json j = Json::array({1, 2, 3});

    assert(ptr(j, "id") == nullptr);
  }

  // ptr(array, index) returns pointer when index exists
  {
    Json arr = Json::array({10, 20, 30});

    const Json *first = ptr(arr, 0);
    const Json *third = ptr(arr, 2);
    const Json *missing = ptr(arr, 3);

    assert(first != nullptr);
    assert(third != nullptr);
    assert(missing == nullptr);

    assert(*first == 10);
    assert(*third == 30);
  }

  // ptr(array, index) returns nullptr when JSON is not an array
  {
    Json j = {
        {"id", 1},
    };

    assert(ptr(j, 0) == nullptr);
  }

  // get_opt<T>(json) returns value when conversion is valid
  {
    Json s = "hello";
    Json i = 42;
    Json b = true;

    auto sv = get_opt<std::string>(s);
    auto iv = get_opt<int>(i);
    auto bv = get_opt<bool>(b);

    assert(sv.has_value());
    assert(iv.has_value());
    assert(bv.has_value());

    assert(*sv == "hello");
    assert(*iv == 42);
    assert(*bv == true);
  }

  // get_opt<T>(json) returns nullopt on invalid type
  {
    Json j = "not an int";

    auto v = get_opt<int>(j);

    assert(!v.has_value());
  }

  // get_opt<T>(json) returns nullopt for null
  {
    Json j = nullptr;

    auto v = get_opt<std::string>(j);

    assert(!v.has_value());
  }

  // get_opt<T>(pointer) returns nullopt when pointer is nullptr
  {
    const Json *jp = nullptr;

    auto v = get_opt<int>(jp);

    assert(!v.has_value());
  }

  // get_opt<T>(obj, key) returns value when key exists and type is valid
  {
    Json j = {
        {"id", 42},
        {"name", "Ada"},
    };

    auto id = get_opt<int>(j, "id");
    auto name = get_opt<std::string>(j, "name");

    assert(id.has_value());
    assert(name.has_value());

    assert(*id == 42);
    assert(*name == "Ada");
  }

  // get_opt<T>(obj, key) returns nullopt when key is missing or invalid
  {
    Json j = {
        {"id", "bad"},
    };

    auto missing = get_opt<int>(j, "missing");
    auto bad = get_opt<int>(j, "id");

    assert(!missing.has_value());
    assert(!bad.has_value());
  }

  // get_opt<T>(array, index) returns value when index exists and type is valid
  {
    Json arr = Json::array({"zero", "one", "two"});

    auto v = get_opt<std::string>(arr, 1);

    assert(v.has_value());
    assert(*v == "one");
  }

  // get_opt<T>(array, index) returns nullopt when index is missing or invalid
  {
    Json arr = Json::array({"zero", "one"});

    auto missing = get_opt<std::string>(arr, 2);
    auto bad = get_opt<int>(arr, 0);

    assert(!missing.has_value());
    assert(!bad.has_value());
  }

  // get_or<T>(json, default) returns value when valid
  {
    Json j = 123;

    int v = get_or<int>(j, -1);

    assert(v == 123);
  }

  // get_or<T>(json, default) returns default when invalid
  {
    Json j = "bad";

    int v = get_or<int>(j, -1);

    assert(v == -1);
  }

  // get_or<T>(obj, key, default) returns value or default
  {
    Json j = {
        {"port", 8080},
        {"host", "localhost"},
    };

    assert(get_or<int>(j, "port", -1) == 8080);
    assert(get_or<int>(j, "missing", -1) == -1);
    assert(get_or<std::string>(j, "host", std::string("none")) == "localhost");
  }

  // get_or<T>(array, index, default) returns value or default
  {
    Json arr = Json::array({10, 20});

    assert(get_or<int>(arr, 0, -1) == 10);
    assert(get_or<int>(arr, 2, -1) == -1);
  }

  // ensure<T>(json) returns value when valid
  {
    Json j = 42;

    int v = ensure<int>(j);

    assert(v == 42);
  }

  // ensure<T>(obj, key) returns value when valid
  {
    Json j = {
        {"id", 7},
        {"name", "Vix.cpp"},
    };

    int id = ensure<int>(j, "id");
    std::string name = ensure<std::string>(j, "name");

    assert(id == 7);
    assert(name == "Vix.cpp");
  }

  // ensure<T>(obj, key) throws when object is not an object
  {
    Json j = Json::array();

    bool thrown = false;

    try
    {
      (void)ensure<int>(j, "id");
    }
    catch (const std::runtime_error &)
    {
      thrown = true;
    }

    assert(thrown);
  }

  // ensure<T>(obj, key) throws when key is missing
  {
    Json j = {
        {"id", 1},
    };

    bool thrown = false;

    try
    {
      (void)ensure<int>(j, "missing");
    }
    catch (const std::runtime_error &)
    {
      thrown = true;
    }

    assert(thrown);
  }

  // ensure<T>(obj, key) throws when type is invalid
  {
    Json j = {
        {"id", "bad"},
    };

    bool thrown = false;

    try
    {
      (void)ensure<int>(j, "id");
    }
    catch (const std::runtime_error &)
    {
      thrown = true;
    }

    assert(thrown);
  }

  // simple_to_json(token) converts primitive values
  {
    token n;
    token b = true;
    token i = 42;
    token d = 3.5;
    token s = "hello";

    assert(simple_to_json(n).is_null());
    assert(simple_to_json(b) == true);
    assert(simple_to_json(i) == 42);
    assert(simple_to_json(d) == 3.5);
    assert(simple_to_json(s) == "hello");
  }

  // simple_to_json(array_t) converts arrays deeply
  {
    array_t arr = array({
        1,
        "two",
        true,
        nullptr,
    });

    Json j = simple_to_json(arr);

    assert(j.is_array());
    assert(j.size() == 4);
    assert(j[0] == 1);
    assert(j[1] == "two");
    assert(j[2] == true);
    assert(j[3].is_null());
  }

  // simple_to_json(kvs) converts objects deeply
  {
    kvs user = simple_obj({
        "name",
        "Ada",
        "age",
        30,
        "active",
        true,
        "skills",
        simple_array({"C++", "JSON"}),
    });

    Json j = simple_to_json(user);

    assert(j.is_object());
    assert(j["name"] == "Ada");
    assert(j["age"] == 30);
    assert(j["active"] == true);

    assert(j["skills"].is_array());
    assert(j["skills"][0] == "C++");
    assert(j["skills"][1] == "JSON");
  }

  // to_json aliases convert Simple values
  {
    token t = "Vix.cpp";
    array_t arr = simple_array({1, 2, 3});
    kvs obj = simple_obj({"runtime", "vix"});

    assert(to_json(t) == "Vix.cpp");

    Json a = to_json(arr);
    assert(a.is_array());
    assert(a.size() == 3);
    assert(a[0] == 1);

    Json o = to_json(obj);
    assert(o.is_object());
    assert(o["runtime"] == "vix");
  }

  return 0;
}
