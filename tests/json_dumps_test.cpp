#include <vix/json/dumps.hpp>
#include <vix/json/loads.hpp>
#include <vix/json/build.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

int main()
{
  using namespace vix::json;

  namespace fs = std::filesystem;

  // dumps() produces pretty JSON with default indent = 2
  {
    Json j = {
        {"name", "Vix.cpp"},
        {"version", "2.6.2"},
        {"active", true},
    };

    const std::string s = dumps(j);

    assert(!s.empty());
    assert(s.find('\n') != std::string::npos);
    assert(s.find("\"name\"") != std::string::npos);
    assert(s.find("\"Vix.cpp\"") != std::string::npos);
    assert(s.find("  ") != std::string::npos);
  }

  // dumps() supports custom indentation
  {
    Json j = {
        {"name", "Vix.cpp"},
        {"features", Json::array({"json", "http"})},
    };

    const std::string s = dumps(j, 4);

    assert(!s.empty());
    assert(s.find('\n') != std::string::npos);
    assert(s.find("    ") != std::string::npos);
    assert(s.find("\"features\"") != std::string::npos);
  }

  // dumps_compact() produces single-line JSON
  {
    Json j = {
        {"name", "Vix.cpp"},
        {"active", true},
        {"count", 3},
    };

    const std::string s = dumps_compact(j);

    assert(!s.empty());
    assert(s.find('\n') == std::string::npos);
    assert(s.find("\"name\"") != std::string::npos);
    assert(s.find("\"Vix.cpp\"") != std::string::npos);
    assert(s.find("\"active\"") != std::string::npos);
    assert(s.find("true") != std::string::npos);
  }

  // dumps_pretty() is equivalent to dumps()
  {
    Json j = {
        {"runtime", "vix"},
        {"module", "json"},
    };

    const std::string a = dumps(j, 2);
    const std::string b = dumps_pretty(j, 2);

    assert(a == b);
  }

  // ensure_ascii=false keeps non-ASCII characters
  {
    Json j = {
        {"city", "Kampala"},
        {"word", "café"},
    };

    const std::string s = dumps_compact(j, false);

    assert(s.find("café") != std::string::npos);
  }

  // ensure_ascii=true escapes non-ASCII characters
  {
    Json j = {
        {"word", "café"},
    };

    const std::string s = dumps_compact(j, true);

    assert(s.find("café") == std::string::npos);
    assert(s.find("\\u") != std::string::npos);
  }

  // dump_file() writes JSON to a file
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_dumps_test";
    const fs::path path = dir / "config.json";

    std::error_code ec;
    fs::remove_all(dir, ec);

    Json j = {
        {"app", "Vix.cpp"},
        {"debug", true},
        {"port", 8080},
    };

    dump_file(path, j);

    assert(fs::exists(path));

    Json loaded = load_file(path);

    assert(loaded.is_object());
    assert(loaded["app"] == "Vix.cpp");
    assert(loaded["debug"] == true);
    assert(loaded["port"] == 8080);

    fs::remove_all(dir, ec);
  }

  // dump_file() creates parent directories
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_dumps_test_nested";
    const fs::path path = dir / "a" / "b" / "config.json";

    std::error_code ec;
    fs::remove_all(dir, ec);

    Json j = {
        {"created", true},
    };

    dump_file(path, j);

    assert(fs::exists(path));
    assert(fs::exists(path.parent_path()));

    Json loaded = load_file(path);

    assert(loaded["created"] == true);

    fs::remove_all(dir, ec);
  }

  // dump_file(const char*) overload works
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_dumps_test_cstr";
    const fs::path path = dir / "out.json";

    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);

    Json j = {
        {"kind", "cstr"},
    };

    const std::string path_string = path.string();
    dump_file(path_string.c_str(), j);

    assert(fs::exists(path));

    Json loaded = load_file(path);
    assert(loaded["kind"] == "cstr");

    fs::remove_all(dir, ec);
  }

  // dump_file(std::string) overload works
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_dumps_test_string";
    const fs::path path = dir / "out.json";

    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);

    Json j = {
        {"kind", "string"},
    };

    dump_file(path.string(), j);

    assert(fs::exists(path));

    Json loaded = load_file(path);
    assert(loaded["kind"] == "string");

    fs::remove_all(dir, ec);
  }

  // dump_file() overwrites an existing file
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_dumps_test_overwrite";
    const fs::path path = dir / "config.json";

    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);

    dump_file(path, Json{{"version", 1}});
    dump_file(path, Json{{"version", 2}});

    Json loaded = load_file(path);

    assert(loaded["version"] == 2);

    fs::remove_all(dir, ec);
  }

  // dump_file() should not leave temp file after success
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_dumps_test_tmp";
    const fs::path path = dir / "config.json";
    const fs::path tmp = path.string() + ".tmp";

    std::error_code ec;
    fs::remove_all(dir, ec);

    Json j = {
        {"ok", true},
    };

    dump_file(path, j);

    assert(fs::exists(path));
    assert(!fs::exists(tmp));

    fs::remove_all(dir, ec);
  }

  return 0;
}
