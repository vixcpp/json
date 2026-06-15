#include <vix/json/json.hpp>

#include <cassert>
#include <string>

int main()
{
  using namespace vix::json;

  // Main module include exposes Json and OrderedJson aliases
  {
    Json j = Json::object();
    OrderedJson oj = OrderedJson::object();

    assert(j.is_object());
    assert(oj.is_object());
  }

  // obj() creates an empty JSON object
  {
    Json j = obj();

    assert(j.is_object());
    assert(j.empty());

    j["name"] = "Vix.cpp";
    assert(j["name"] == "Vix.cpp");
  }

  // arr() creates an empty JSON array
  {
    Json j = arr();

    assert(j.is_array());
    assert(j.empty());

    j.push_back(1);
    j.push_back("two");

    assert(j.size() == 2);
    assert(j[0] == 1);
    assert(j[1] == "two");
  }

  // build helpers are available from the module include
  {
    auto j = o(
        "name", "Vix.cpp",
        "features", a("parse", "dump", "jpath"));

    assert(j.is_object());
    assert(j["name"] == "Vix.cpp");

    assert(j["features"].is_array());
    assert(j["features"][0] == "parse");
    assert(j["features"][1] == "dump");
    assert(j["features"][2] == "jpath");
  }

  // loads() and try_loads() are available from the module include
  {
    Json j = loads(R"({"id": 42, "name": "Ada"})");

    assert(j.is_object());
    assert(j["id"] == 42);
    assert(j["name"] == "Ada");

    auto bad = try_loads("{bad json");
    assert(!bad.has_value());
  }

  // dumps helpers are available from the module include
  {
    Json j = {
        {"runtime", "vix"},
        {"module", "json"},
    };

    std::string pretty = dumps(j);
    std::string compact = dumps_compact(j);

    assert(pretty.find('\n') != std::string::npos);
    assert(compact.find('\n') == std::string::npos);
    assert(compact.find("\"runtime\"") != std::string::npos);
  }

  // jpath helpers are available from the module include
  {
    Json j = obj();

    bool ok = jset(j, "user.profile.name", "Gaspard");

    assert(ok);
    assert(j["user"]["profile"]["name"] == "Gaspard");

    const Json *name = jget(static_cast<const Json &>(j), "user.profile.name");

    assert(name != nullptr);
    assert(*name == "Gaspard");
  }

  // convert helpers are available from the module include
  {
    Json j = {
        {"id", 7},
        {"name", "Softadastra"},
    };

    auto id = get_opt<int>(j, "id");
    std::string name = get_or<std::string>(j, "name", std::string("unknown"));

    assert(id.has_value());
    assert(*id == 7);
    assert(name == "Softadastra");

    int strict_id = ensure<int>(j, "id");
    assert(strict_id == 7);
  }

  // Simple conversion helpers are available through convert.hpp/json.hpp
  {
    token t = "hello";
    Json j = to_json(t);

    assert(j == "hello");
  }

  return 0;
}
