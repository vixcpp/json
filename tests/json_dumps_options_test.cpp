#include <vix/json/dumps.hpp>
#include <vix/json/loads.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

int main()
{
  using namespace vix::json;

  namespace fs = std::filesystem;

  // SerializeOptions pretty=false produces compact single-line output.
  {
    Json j = {
        {"name", "Vix.cpp"},
        {"active", true},
        {"count", 3},
    };

    const std::string s = dumps(j, SerializeOptions{false, 8, false});

    assert(!s.empty());
    assert(s.find('\n') == std::string::npos);
    assert(s.find("\"name\"") != std::string::npos);
    assert(s.find("\"active\"") != std::string::npos);
    assert(s.find("\"count\"") != std::string::npos);
  }

  // SerializeOptions pretty=true respects indentation width.
  {
    Json j = {
        {"meta",
         {
             {"name", "Vix.cpp"},
             {"version", 1},
         }},
    };

    const std::string s = dumps(j, SerializeOptions{true, 4, false});

    assert(!s.empty());
    assert(s.find('\n') != std::string::npos);
    assert(s.find("\n    \"meta\"") != std::string::npos);
    assert(s.find("\n        \"name\"") != std::string::npos);
  }

  // SerializeOptions ensure_ascii=false keeps non-ASCII text.
  {
    Json j = {
        {"word", "café"},
    };

    const std::string s = dumps(j, SerializeOptions{false, 2, false});

    assert(s.find("café") != std::string::npos);
  }

  // SerializeOptions ensure_ascii=true escapes non-ASCII text.
  {
    Json j = {
        {"word", "café"},
    };

    const std::string s = dumps(j, SerializeOptions{false, 2, true});

    assert(s.find("café") == std::string::npos);
    assert(s.find("\\u00e9") != std::string::npos);
  }

  // Negative indentation is invalid for the options API.
  {
    Json j = {
        {"ok", true},
    };

    bool threw = false;

    try
    {
      (void)dumps(j, SerializeOptions{true, -1, false});
    }
    catch (const std::invalid_argument &)
    {
      threw = true;
    }

    assert(threw);
  }

  // dump_file() with SerializeOptions uses the same compact semantics as dumps().
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_dumps_options_test_compact";
    const fs::path path = dir / "config.json";

    std::error_code ec;
    fs::remove_all(dir, ec);

    Json j = {
        {"message", "hello"},
        {"value", 7},
    };

    const SerializeOptions options{false, 6, false};
    const std::string expected = dumps(j, options);

    dump_file(path, j, options);

    assert(fs::exists(path));

    std::ifstream ifs(path, std::ios::binary);
    assert(ifs);

    const std::string content((std::istreambuf_iterator<char>(ifs)),
                              std::istreambuf_iterator<char>());

    assert(content == expected);
    assert(content.find('\n') == std::string::npos);

    fs::remove_all(dir, ec);
  }

  // dump_file() with SerializeOptions uses the same pretty semantics as dumps().
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_dumps_options_test_pretty";
    const fs::path path = dir / "config.json";

    std::error_code ec;
    fs::remove_all(dir, ec);

    Json j = {
        {"outer",
         {
             {"inner", "value"},
         }},
    };

    const SerializeOptions options{true, 3, false};
    const std::string expected = dumps(j, options);

    dump_file(path, j, options);

    assert(fs::exists(path));

    std::ifstream ifs(path, std::ios::binary);
    assert(ifs);

    const std::string content((std::istreambuf_iterator<char>(ifs)),
                              std::istreambuf_iterator<char>());

    assert(content == expected);
    assert(content.find('\n') != std::string::npos);
    assert(content.find("\n   \"outer\"") != std::string::npos);

    fs::remove_all(dir, ec);
  }

  // dump_file() rejects invalid SerializeOptions clearly.
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_dumps_options_test_invalid";
    const fs::path path = dir / "config.json";

    std::error_code ec;
    fs::remove_all(dir, ec);

    Json j = {
        {"ok", true},
    };

    bool threw = false;

    try
    {
      dump_file(path, j, SerializeOptions{true, -1, false});
    }
    catch (const std::invalid_argument &)
    {
      threw = true;
    }

    assert(threw);
    assert(!fs::exists(path));

    fs::remove_all(dir, ec);
  }

  // Legacy positional dumps(j, -1) remains compact.
  {
    Json j = {
        {"legacy", true},
        {"value", 9},
    };

    const std::string s = dumps(j, -1);

    assert(!s.empty());
    assert(s.find('\n') == std::string::npos);
    assert(s.find("\"legacy\"") != std::string::npos);
    assert(s.find("\"value\"") != std::string::npos);
  }

  // Legacy positional dump_file(path, j, -1) writes compact content.
  {
    const fs::path dir = fs::temp_directory_path() / "vix_json_dumps_options_test_legacy_compact";
    const fs::path path = dir / "config.json";

    std::error_code ec;
    fs::remove_all(dir, ec);

    Json j = {
        {"legacy", true},
        {"value", 9},
    };

    dump_file(path, j, -1);

    assert(fs::exists(path));

    std::ifstream ifs(path, std::ios::binary);
    assert(ifs);

    const std::string content((std::istreambuf_iterator<char>(ifs)),
                              std::istreambuf_iterator<char>());

    assert(!content.empty());
    assert(content.find('\n') == std::string::npos);
    assert(content.find("\"legacy\"") != std::string::npos);
    assert(content.find("\"value\"") != std::string::npos);

    fs::remove_all(dir, ec);
  }

  return 0;
}
