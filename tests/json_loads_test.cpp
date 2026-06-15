#include <vix/json/loads.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>

int main()
{
  using namespace vix::json;

  namespace fs = std::filesystem;

  // loads(std::string_view) parses valid JSON object
  {
    Json j = loads(R"({"id": 42, "name": "Gaspard", "active": true})");

    assert(j.is_object());
    assert(j["id"] == 42);
    assert(j["name"] == "Gaspard");
    assert(j["active"] == true);
  }

  // loads(std::string_view) parses valid JSON array
  {
    Json j = loads(R"([1, 2, 3, "hello", true, null])");

    assert(j.is_array());
    assert(j.size() == 6);
    assert(j[0] == 1);
    assert(j[1] == 2);
    assert(j[2] == 3);
    assert(j[3] == "hello");
    assert(j[4] == true);
    assert(j[5].is_null());
  }

  // loads(const char*) overload works
  {
    Json j = loads(R"({"runtime": "vix", "module": "json"})");

    assert(j.is_object());
    assert(j["runtime"] == "vix");
    assert(j["module"] == "json");
  }

  // loads() throws on invalid JSON
  {
    bool thrown = false;

    try
    {
      (void)loads("{bad json");
    }
    catch (const nlohmann::json::parse_error &)
    {
      thrown = true;
    }

    assert(thrown);
  }

  // try_loads(std::string_view) returns value for valid JSON
  {
    auto maybe = try_loads(R"({"ok": true, "count": 3})");

    assert(maybe.has_value());
    assert((*maybe)["ok"] == true);
    assert((*maybe)["count"] == 3);
  }

  // try_loads(std::string_view) returns nullopt for invalid JSON
  {
    auto maybe = try_loads("{bad json");

    assert(!maybe.has_value());
  }

  // try_loads(const char*) overload works
  {
    auto maybe = try_loads(R"({"name": "Vix.cpp"})");

    assert(maybe.has_value());
    assert((*maybe)["name"] == "Vix.cpp");
  }

  // try_loads(const char*) overload returns nullopt for invalid JSON
  {
    auto maybe = try_loads("[1, 2,");

    assert(!maybe.has_value());
  }

  // load_file(fs::path) loads a valid JSON file
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_loads_test";
    const fs::path path = dir / "config.json";

    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);

    {
      std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
      ofs << R"({"app": "Vix.cpp", "port": 8080, "debug": true})";
    }

    Json j = load_file(path);

    assert(j.is_object());
    assert(j["app"] == "Vix.cpp");
    assert(j["port"] == 8080);
    assert(j["debug"] == true);

    fs::remove_all(dir, ec);
  }

  // load_file(const char*) overload works
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_loads_test_cstr";
    const fs::path path = dir / "config.json";

    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);

    {
      std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
      ofs << R"({"kind": "cstr"})";
    }

    const std::string path_string = path.string();
    Json j = load_file(path_string.c_str());

    assert(j.is_object());
    assert(j["kind"] == "cstr");

    fs::remove_all(dir, ec);
  }

  // load_file() throws when file does not exist
  {
    const fs::path path = fs::temp_directory_path() / "vix_json_loads_missing_file.json";

    std::error_code ec;
    fs::remove(path, ec);

    bool thrown = false;

    try
    {
      (void)load_file(path);
    }
    catch (const std::runtime_error &)
    {
      thrown = true;
    }

    assert(thrown);
  }

  // load_file() throws when file is empty
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_loads_test_empty";
    const fs::path path = dir / "empty.json";

    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);

    {
      std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    }

    bool thrown = false;

    try
    {
      (void)load_file(path);
    }
    catch (const std::runtime_error &)
    {
      thrown = true;
    }

    assert(thrown);

    fs::remove_all(dir, ec);
  }

  // load_file() throws parse_error when file contains invalid JSON
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_loads_test_invalid";
    const fs::path path = dir / "bad.json";

    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);

    {
      std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
      ofs << "{bad json";
    }

    bool thrown = false;

    try
    {
      (void)load_file(path);
    }
    catch (const nlohmann::json::parse_error &)
    {
      thrown = true;
    }

    assert(thrown);

    fs::remove_all(dir, ec);
  }

  // try_load_file(fs::path) returns value for valid JSON file
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_try_load_file_test";
    const fs::path path = dir / "config.json";

    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);

    {
      std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
      ofs << R"({"loaded": true})";
    }

    auto maybe = try_load_file(path);

    assert(maybe.has_value());
    assert((*maybe)["loaded"] == true);

    fs::remove_all(dir, ec);
  }

  // try_load_file(fs::path) returns nullopt for missing file
  {
    const fs::path path = fs::temp_directory_path() / "vix_json_try_load_file_missing.json";

    std::error_code ec;
    fs::remove(path, ec);

    auto maybe = try_load_file(path);

    assert(!maybe.has_value());
  }

  // try_load_file(fs::path) returns nullopt for empty file
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_try_load_file_empty";
    const fs::path path = dir / "empty.json";

    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);

    {
      std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    }

    auto maybe = try_load_file(path);

    assert(!maybe.has_value());

    fs::remove_all(dir, ec);
  }

  // try_load_file(fs::path) returns nullopt for invalid JSON file
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_try_load_file_invalid";
    const fs::path path = dir / "bad.json";

    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);

    {
      std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
      ofs << "{bad json";
    }

    auto maybe = try_load_file(path);

    assert(!maybe.has_value());

    fs::remove_all(dir, ec);
  }

  // try_load_file(const char*) overload works
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_try_load_file_cstr";
    const fs::path path = dir / "config.json";

    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);

    {
      std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
      ofs << R"({"kind": "try_cstr"})";
    }

    const std::string path_string = path.string();
    auto maybe = try_load_file(path_string.c_str());

    assert(maybe.has_value());
    assert((*maybe)["kind"] == "try_cstr");

    fs::remove_all(dir, ec);
  }

  return 0;
}
