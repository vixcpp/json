#include <vix/json/jpath.hpp>
#include <vix/json/build.hpp>

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

int main()
{
  using namespace vix::json;

  // tokenize_path() parses dot notation
  {
    std::vector<Token> toks = tokenize_path("user.profile.name");

    assert(toks.size() == 3);

    assert(toks[0].kind == Token::Key);
    assert(toks[0].key == "user");

    assert(toks[1].kind == Token::Key);
    assert(toks[1].key == "profile");

    assert(toks[2].kind == Token::Key);
    assert(toks[2].key == "name");
  }

  // tokenize_path() parses array indices
  {
    std::vector<Token> toks = tokenize_path("users[0].email");

    assert(toks.size() == 3);

    assert(toks[0].kind == Token::Key);
    assert(toks[0].key == "users");

    assert(toks[1].kind == Token::Index);
    assert(toks[1].index == 0);

    assert(toks[2].kind == Token::Key);
    assert(toks[2].key == "email");
  }

  // tokenize_path() parses quoted keys
  {
    std::vector<Token> toks = tokenize_path(R"(["complex.key"].value)");

    assert(toks.size() == 2);

    assert(toks[0].kind == Token::Key);
    assert(toks[0].key == "complex.key");

    assert(toks[1].kind == Token::Key);
    assert(toks[1].key == "value");
  }

  // tokenize_path() parses quoted keys with spaces
  {
    std::vector<Token> toks = tokenize_path(R"(["a b c"][0])");

    assert(toks.size() == 2);

    assert(toks[0].kind == Token::Key);
    assert(toks[0].key == "a b c");

    assert(toks[1].kind == Token::Index);
    assert(toks[1].index == 0);
  }

  // tokenize_path() parses escaped quote in quoted key
  {
    std::vector<Token> toks = tokenize_path(R"(["a\"b"])");

    assert(toks.size() == 1);
    assert(toks[0].kind == Token::Key);
    assert(toks[0].key == "a\"b");
  }

  // tokenize_path() parses escaped backslash in quoted key
  {
    std::vector<Token> toks = tokenize_path(R"(["a\\b"])");

    assert(toks.size() == 1);
    assert(toks[0].kind == Token::Key);
    assert(toks[0].key == "a\\b");
  }

  // tokenize_path() throws on empty key segment
  {
    bool thrown = false;

    try
    {
      (void)tokenize_path("user..name");
    }
    catch (const std::runtime_error &)
    {
      thrown = true;
    }

    assert(thrown);
  }

  // tokenize_path() throws on missing closing bracket
  {
    bool thrown = false;

    try
    {
      (void)tokenize_path("users[0");
    }
    catch (const std::runtime_error &)
    {
      thrown = true;
    }

    assert(thrown);
  }

  // tokenize_path() throws on bad array index
  {
    bool thrown = false;

    try
    {
      (void)tokenize_path("users[abc]");
    }
    catch (const std::runtime_error &)
    {
      thrown = true;
    }

    assert(thrown);
  }

  // tokenize_path() throws on signed array index
  {
    bool thrown = false;

    try
    {
      (void)tokenize_path("users[-1]");
    }
    catch (const std::runtime_error &)
    {
      thrown = true;
    }

    assert(thrown);
  }

  // const jget() reads a nested object value
  {
    Json j = {
        {"user",
         {
             {"profile",
              {
                  {"name", "Gaspard"},
                  {"age", 30},
              }},
         }},
    };

    const Json &cj = j;

    const Json *name = jget(cj, "user.profile.name");
    const Json *age = jget(cj, "user.profile.age");

    assert(name != nullptr);
    assert(age != nullptr);

    assert(*name == "Gaspard");
    assert(*age == 30);
  }

  // const jget() reads array values
  {
    Json j = {
        {"users",
         Json::array({
             Json{{"email", "a@example.com"}},
             Json{{"email", "b@example.com"}},
         })},
    };

    const Json &cj = j;

    const Json *email = jget(cj, "users[1].email");

    assert(email != nullptr);
    assert(*email == "b@example.com");
  }

  // const jget() reads quoted keys
  {
    Json j = {
        {"complex.key",
         {
             {"value", 42},
         }},
        {"a b c", Json::array({"first"})},
    };

    const Json &cj = j;

    const Json *value = jget(cj, R"(["complex.key"].value)");
    const Json *first = jget(cj, R"(["a b c"][0])");

    assert(value != nullptr);
    assert(first != nullptr);

    assert(*value == 42);
    assert(*first == "first");
  }

  // const jget() returns nullptr for missing key
  {
    Json j = {
        {"user",
         {
             {"name", "Ada"},
         }},
    };

    const Json &cj = j;

    assert(jget(cj, "user.email") == nullptr);
  }

  // const jget() returns nullptr for out-of-range index
  {
    Json j = {
        {"items", Json::array({1, 2})},
    };

    const Json &cj = j;

    assert(jget(cj, "items[2]") == nullptr);
  }

  // const jget() returns nullptr when path expects object but value is not object
  {
    Json j = {
        {"user", "not object"},
    };

    const Json &cj = j;

    assert(jget(cj, "user.name") == nullptr);
  }

  // const jget() returns nullptr when path expects array but value is not array
  {
    Json j = {
        {"items", "not array"},
    };

    const Json &cj = j;

    assert(jget(cj, "items[0]") == nullptr);
  }

  // const jget() returns nullptr for invalid syntax
  {
    Json j = {
        {"items", Json::array({1})},
    };

    const Json &cj = j;

    assert(jget(cj, "items[bad]") == nullptr);
    assert(jget(cj, "items[0") == nullptr);
  }

  // mutable jget() creates missing objects
  {
    Json j = Json::object();

    Json *name = jget(j, "user.profile.name");
    assert(name != nullptr);

    *name = "Gaspard";

    assert(j.is_object());
    assert(j["user"].is_object());
    assert(j["user"]["profile"].is_object());
    assert(j["user"]["profile"]["name"] == "Gaspard");
  }

  // mutable jget() creates missing arrays and grows with nulls
  {
    Json j = Json::object();

    Json *item = jget(j, "items[2].name");
    assert(item != nullptr);

    *item = "third";

    assert(j["items"].is_array());
    assert(j["items"].size() == 3);

    assert(j["items"][0].is_null());
    assert(j["items"][1].is_null());

    assert(j["items"][2].is_object());
    assert(j["items"][2]["name"] == "third");
  }

  // mutable jget() replaces incompatible intermediate values
  {
    Json j = {
        {"user", "old value"},
        {"items", "old value"},
    };

    Json *name = jget(j, "user.name");
    Json *first = jget(j, "items[0]");

    assert(name != nullptr);
    assert(first != nullptr);

    *name = "Ada";
    *first = 123;

    assert(j["user"].is_object());
    assert(j["user"]["name"] == "Ada");

    assert(j["items"].is_array());
    assert(j["items"][0] == 123);
  }

  // mutable jget() throws on invalid syntax
  {
    Json j = Json::object();

    bool thrown = false;

    try
    {
      (void)jget(j, "items[bad]");
    }
    catch (const std::runtime_error &)
    {
      thrown = true;
    }

    assert(thrown);
  }

  // jset() writes a simple nested value
  {
    Json j = Json::object();

    bool ok = jset(j, "user.name", "Gaspard");

    assert(ok);
    assert(j["user"]["name"] == "Gaspard");
  }

  // jset() writes an array value
  {
    Json j = Json::object();

    bool ok = jset(j, "users[0].email", "ada@example.com");

    assert(ok);
    assert(j["users"].is_array());
    assert(j["users"][0]["email"] == "ada@example.com");
  }

  // jset() writes quoted keys
  {
    Json j = Json::object();

    bool ok1 = jset(j, R"(["complex.key"].value)", 42);
    bool ok2 = jset(j, R"(["a b c"][0])", "first");

    assert(ok1);
    assert(ok2);

    assert(j["complex.key"]["value"] == 42);
    assert(j["a b c"].is_array());
    assert(j["a b c"][0] == "first");
  }

  // jset() returns false on invalid path
  {
    Json j = Json::object();

    bool ok1 = jset(j, "items[bad]", 1);
    bool ok2 = jset(j, "items[0", 1);

    assert(!ok1);
    assert(!ok2);
  }

  // jget(const char*) overload works
  {
    Json j = {
        {"user",
         {
             {"name", "Vix.cpp"},
         }},
    };

    const Json &cj = j;

    const Json *name = jget(cj, "user.name");

    assert(name != nullptr);
    assert(*name == "Vix.cpp");
  }

  // jget(Json&, const char*) overload works
  {
    Json j = Json::object();

    Json *name = jget(j, "user.name");

    assert(name != nullptr);

    *name = "Softadastra";

    assert(j["user"]["name"] == "Softadastra");
  }

  // jset(const char*) overload works
  {
    Json j = Json::object();

    bool ok = jset(j, "config.port", 8080);

    assert(ok);
    assert(j["config"]["port"] == 8080);
  }

  return 0;
}
