#include <vix/json/Simple.hpp>

#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

int main()
{
  using namespace vix::json;

  // token default is null
  {
    token t;

    assert(t.is_null());
    assert(!t.is_bool());
    assert(!t.is_i64());
    assert(!t.is_f64());
    assert(!t.is_string());
    assert(!t.is_array());
    assert(!t.is_object());
  }

  // token supports nullptr
  {
    token t = nullptr;

    assert(t.is_null());
  }

  // token supports bool
  {
    token t = true;

    assert(t.is_bool());
    assert(t.as_bool() != nullptr);
    assert(*t.as_bool() == true);
    assert(t.as_bool_or(false) == true);
  }

  // token supports int and stores as int64
  {
    token t = 42;

    assert(t.is_i64());
    assert(t.as_i64() != nullptr);
    assert(*t.as_i64() == 42);
    assert(t.as_i64_or(-1) == 42);
  }

  // token supports int64
  {
    std::int64_t value = 9000000000LL;
    token t = value;

    assert(t.is_i64());
    assert(t.as_i64() != nullptr);
    assert(*t.as_i64() == value);
  }

  // token supports long long without ambiguity
  {
    long long value = 123456789LL;
    token t = value;

    assert(t.is_i64());
    assert(t.as_i64() != nullptr);
    assert(*t.as_i64() == static_cast<std::int64_t>(value));
  }

  // token supports unsigned long long without ambiguity
  {
    unsigned long long value = 123456789ULL;
    token t = value;

    assert(t.is_i64());
    assert(t.as_i64() != nullptr);
    assert(*t.as_i64() == static_cast<std::int64_t>(value));
  }

  // token supports double
  {
    token t = 3.5;

    assert(t.is_f64());
    assert(t.as_f64() != nullptr);
    assert(*t.as_f64() == 3.5);
    assert(t.as_f64_or(0.0) == 3.5);
  }

  // token supports const char*
  {
    token t = "hello";

    assert(t.is_string());
    assert(t.as_string() != nullptr);
    assert(*t.as_string() == "hello");
    assert(t.as_string_or("bad") == "hello");
  }

  // token supports nullptr C string as empty string
  {
    const char *s = nullptr;
    token t = s;

    assert(t.is_string());
    assert(t.as_string() != nullptr);
    assert(*t.as_string() == "");
  }

  // token supports std::string
  {
    token t = std::string("Vix.cpp");

    assert(t.is_string());
    assert(t.as_string() != nullptr);
    assert(*t.as_string() == "Vix.cpp");
  }

  // token default getters return fallback for wrong type
  {
    token t = "not an int";

    assert(t.as_bool_or(true) == true);
    assert(t.as_i64_or(-1) == -1);
    assert(t.as_f64_or(2.5) == 2.5);
    assert(t.as_string_or("fallback") == "not an int");
  }

  // token setters work
  {
    token t;

    t.set_bool(true);
    assert(t.is_bool());
    assert(*t.as_bool() == true);

    t.set_int(12);
    assert(t.is_i64());
    assert(*t.as_i64() == 12);

    t.set_i64(99);
    assert(t.is_i64());
    assert(*t.as_i64() == 99);

    t.set_ll(123LL);
    assert(t.is_i64());
    assert(*t.as_i64() == 123);

    t.set_ull(456ULL);
    assert(t.is_i64());
    assert(*t.as_i64() == 456);

    t.set_f64(1.25);
    assert(t.is_f64());
    assert(*t.as_f64() == 1.25);

    t.set_string("hello");
    assert(t.is_string());
    assert(*t.as_string() == "hello");

    t.set_cstr("world");
    assert(t.is_string());
    assert(*t.as_string() == "world");

    t.set_cstr(nullptr);
    assert(t.is_string());
    assert(*t.as_string() == "");

    t.set_null();
    assert(t.is_null());
  }

  // array_t initializer_list and basic access
  {
    array_t arr = array({1, "two", true, nullptr});

    assert(arr.size() == 4);
    assert(!arr.empty());

    assert(arr[0].is_i64());
    assert(*arr[0].as_i64() == 1);

    assert(arr[1].is_string());
    assert(*arr[1].as_string() == "two");

    assert(arr[2].is_bool());
    assert(*arr[2].as_bool() == true);

    assert(arr[3].is_null());
  }

  // array_t push helpers
  {
    array_t arr;

    arr.push_null();
    arr.push_bool(true);
    arr.push_int(1);
    arr.push_i64(2);
    arr.push_ll(3LL);
    arr.push_ull(4ULL);
    arr.push_f64(5.5);
    arr.push_string("hello");
    arr.push_cstr("world");

    assert(arr.size() == 9);

    assert(arr[0].is_null());
    assert(*arr[1].as_bool() == true);
    assert(*arr[2].as_i64() == 1);
    assert(*arr[3].as_i64() == 2);
    assert(*arr[4].as_i64() == 3);
    assert(*arr[5].as_i64() == 4);
    assert(*arr[6].as_f64() == 5.5);
    assert(*arr[7].as_string() == "hello");
    assert(*arr[8].as_string() == "world");
  }

  // array_t ensure grows with null values
  {
    array_t arr;

    token &t = arr.ensure(3);
    t = "last";

    assert(arr.size() == 4);
    assert(arr[0].is_null());
    assert(arr[1].is_null());
    assert(arr[2].is_null());
    assert(arr[3].is_string());
    assert(*arr[3].as_string() == "last");
  }

  // array_t erase_at removes element and preserves order
  {
    array_t arr = array({1, 2, 3});

    assert(arr.erase_at(1));
    assert(arr.size() == 2);
    assert(*arr[0].as_i64() == 1);
    assert(*arr[1].as_i64() == 3);

    assert(!arr.erase_at(10));
  }

  // array_t resize fills new elements with null
  {
    array_t arr;

    arr.resize(3);

    assert(arr.size() == 3);
    assert(arr[0].is_null());
    assert(arr[1].is_null());
    assert(arr[2].is_null());
  }

  // array_t resize with fill value
  {
    array_t arr;

    arr.resize(3, token("x"));

    assert(arr.size() == 3);
    assert(*arr[0].as_string() == "x");
    assert(*arr[1].as_string() == "x");
    assert(*arr[2].as_string() == "x");
  }

  // array_t clear and reserve
  {
    array_t arr;

    arr.reserve(10);
    assert(arr.capacity() >= 10);

    arr.push_int(1);
    arr.clear();

    assert(arr.empty());
    assert(arr.size() == 0);
  }

  // kvs initializer_list creates flattened key/value storage
  {
    kvs obj = simple_obj({
        "name",
        "Ada",
        "age",
        30,
        "active",
        true,
    });

    assert(!obj.empty());
    assert(obj.raw_size() == 6);
    assert(obj.size_pairs() == 3);

    assert(obj.contains("name"));
    assert(obj.contains("age"));
    assert(obj.contains("active"));

    assert(obj.get_string("name").has_value());
    assert(*obj.get_string("name") == "Ada");

    assert(obj.get_i64("age").has_value());
    assert(*obj.get_i64("age") == 30);

    assert(obj.get_bool("active").has_value());
    assert(*obj.get_bool("active") == true);
  }

  // kvs operator[] creates missing key with null
  {
    kvs obj;

    token &value = obj["name"];

    assert(value.is_null());
    assert(obj.contains("name"));

    value = "Gaspard";

    assert(obj.get_string("name").has_value());
    assert(*obj.get_string("name") == "Gaspard");
  }

  // kvs set replaces existing value
  {
    kvs obj;

    obj.set("count", 1);
    obj.set("count", 2);

    assert(obj.size_pairs() == 1);
    assert(obj.get_i64("count").has_value());
    assert(*obj.get_i64("count") == 2);
  }

  // kvs typed setters
  {
    kvs obj;

    obj.set_string("name", "Vix.cpp");
    obj.set_bool("active", true);
    obj.set_f64("score", 9.5);
    obj.set_i64("id", 42);
    obj.set_int("port", 8080);
    obj.set_ll("big", 123LL);
    obj.set_ull("unsigned", 456ULL);

    assert(*obj.get_string("name") == "Vix.cpp");
    assert(*obj.get_bool("active") == true);
    assert(*obj.get_f64("score") == 9.5);
    assert(*obj.get_i64("id") == 42);
    assert(*obj.get_i64("port") == 8080);
    assert(*obj.get_i64("big") == 123);
    assert(*obj.get_i64("unsigned") == 456);
  }

  // kvs typed getters return nullopt for missing or wrong type
  {
    kvs obj = simple_obj({
        "name",
        "Ada",
        "age",
        30,
    });

    assert(!obj.get_string("missing").has_value());
    assert(!obj.get_i64("name").has_value());
    assert(!obj.get_bool("age").has_value());
    assert(!obj.get_f64("name").has_value());
  }

  // kvs typed getters with defaults
  {
    kvs obj = simple_obj({
        "name",
        "Ada",
        "age",
        30,
        "score",
        8.5,
        "active",
        true,
    });

    assert(obj.get_string_or("name", "none") == "Ada");
    assert(obj.get_string_or("missing", "none") == "none");

    assert(obj.get_i64_or("age", -1) == 30);
    assert(obj.get_i64_or("missing", -1) == -1);

    assert(obj.get_f64_or("score", -1.0) == 8.5);
    assert(obj.get_f64_or("missing", -1.0) == -1.0);

    assert(obj.get_bool_or("active", false) == true);
    assert(obj.get_bool_or("missing", true) == true);
  }

  // kvs default getters without explicit default
  {
    kvs obj;

    assert(obj.get_string_or("missing") == "");
    assert(obj.get_i64_or("missing") == 0);
    assert(obj.get_f64_or("missing") == 0.0);
    assert(obj.get_bool_or("missing") == false);
  }

  // kvs erase removes key/value pair
  {
    kvs obj = simple_obj({
        "a",
        1,
        "b",
        2,
        "c",
        3,
    });

    assert(obj.erase("b"));
    assert(!obj.contains("b"));
    assert(obj.size_pairs() == 2);

    assert(!obj.erase("missing"));
  }

  // kvs erase_if removes matching pairs
  {
    kvs obj = simple_obj({
        "keep",
        1,
        "remove_a",
        2,
        "remove_b",
        3,
    });

    std::size_t removed = obj.erase_if([](std::string_view key, const token &)
                                       { return key.find("remove_") == 0; });

    assert(removed == 2);
    assert(obj.contains("keep"));
    assert(!obj.contains("remove_a"));
    assert(!obj.contains("remove_b"));
    assert(obj.size_pairs() == 1);
  }

  // kvs keys returns string keys
  {
    kvs obj = simple_obj({
        "first",
        1,
        "second",
        2,
    });

    std::vector<std::string> keys = obj.keys();

    assert(keys.size() == 2);
    assert(keys[0] == "first");
    assert(keys[1] == "second");
  }

  // kvs for_each_pair iterates string-key pairs
  {
    kvs obj = simple_obj({
        "a",
        1,
        "b",
        2,
    });

    int sum = 0;

    obj.for_each_pair([&](std::string_view, const token &value)
                      { sum += static_cast<int>(value.as_i64_or(0)); });

    assert(sum == 3);
  }

  // kvs for_each_pair ignores non-string keys
  {
    kvs obj;
    obj.push_pair(token(123), token("bad"));
    obj.push_pair(token("good"), token(42));

    int count = 0;

    obj.for_each_pair([&](std::string_view key, const token &value)
                      {
                        ++count;
                        assert(key == "good");
                        assert(value.as_i64_or(0) == 42); });

    assert(count == 1);
  }

  // kvs merge_from with overwrite=true replaces existing keys
  {
    kvs base = simple_obj({
        "name",
        "old",
        "keep",
        true,
    });

    kvs other = simple_obj({
        "name",
        "new",
        "age",
        30,
    });

    std::size_t changes = base.merge_from(other, true);

    assert(changes == 2);
    assert(base.get_string_or("name") == "new");
    assert(base.get_bool_or("keep") == true);
    assert(base.get_i64_or("age") == 30);
  }

  // kvs merge_from with overwrite=false keeps existing keys
  {
    kvs base = simple_obj({
        "name",
        "old",
    });

    kvs other = simple_obj({
        "name",
        "new",
        "age",
        30,
    });

    std::size_t changes = base.merge_from(other, false);

    assert(changes == 1);
    assert(base.get_string_or("name") == "old");
    assert(base.get_i64_or("age") == 30);
  }

  // kvs ensure_object creates nested object
  {
    kvs root;

    kvs &user = root.ensure_object("user");
    user.set_string("name", "Ada");

    const token *user_token = root.get_ptr("user");

    assert(user_token != nullptr);
    assert(user_token->is_object());

    auto user_ptr = user_token->as_object_ptr();

    assert(user_ptr != nullptr);
    assert(user_ptr->get_string_or("name") == "Ada");
  }

  // kvs ensure_array creates nested array
  {
    kvs root;

    array_t &items = root.ensure_array("items");
    items.push_string("first");

    const token *items_token = root.get_ptr("items");

    assert(items_token != nullptr);
    assert(items_token->is_array());

    auto items_ptr = items_token->as_array_ptr();

    assert(items_ptr != nullptr);
    assert(items_ptr->size() == 1);
    assert(items_ptr->at(0).as_string_or("") == "first");
  }

  // token can hold array_t
  {
    array_t arr = simple_array({1, 2, 3});
    token t = arr;

    assert(t.is_array());

    auto p = t.as_array_ptr();

    assert(p != nullptr);
    assert(p->size() == 3);
    assert(p->at(0).as_i64_or(0) == 1);
  }

  // token can hold kvs
  {
    kvs obj = simple_obj({
        "name",
        "Vix.cpp",
    });

    token t = obj;

    assert(t.is_object());

    auto p = t.as_object_ptr();

    assert(p != nullptr);
    assert(p->get_string_or("name") == "Vix.cpp");
  }

  // token ensure_array replaces non-array value
  {
    token t = "not array";

    array_t &arr = t.ensure_array();
    arr.push_int(7);

    assert(t.is_array());

    auto p = t.as_array_ptr();

    assert(p != nullptr);
    assert(p->size() == 1);
    assert(p->at(0).as_i64_or(0) == 7);
  }

  // token ensure_object replaces non-object value
  {
    token t = "not object";

    kvs &obj = t.ensure_object();
    obj.set_string("name", "Softadastra");

    assert(t.is_object());

    auto p = t.as_object_ptr();

    assert(p != nullptr);
    assert(p->get_string_or("name") == "Softadastra");
  }

  // set_array and set_object work
  {
    token t;

    array_t arr = simple_array({"a", "b"});
    t.set_array(arr);

    assert(t.is_array());
    assert(t.as_array_ptr() != nullptr);
    assert(t.as_array_ptr()->size() == 2);

    kvs obj = simple_obj({
        "kind",
        "object",
    });

    t.set_object(obj);

    assert(t.is_object());
    assert(t.as_object_ptr() != nullptr);
    assert(t.as_object_ptr()->get_string_or("kind") == "object");
  }

  // builders from vector work
  {
    std::vector<token> arr_values;
    arr_values.emplace_back(1);
    arr_values.emplace_back(2);

    array_t arr = simple_array(arr_values);

    assert(arr.size() == 2);
    assert(arr[0].as_i64_or(0) == 1);
    assert(arr[1].as_i64_or(0) == 2);

    std::vector<token> obj_values;
    obj_values.emplace_back("name");
    obj_values.emplace_back("Ada");

    kvs obj = simple_obj(obj_values);

    assert(obj.size_pairs() == 1);
    assert(obj.get_string_or("name") == "Ada");
  }

  // clear and reserve_pairs
  {
    kvs obj;

    obj.reserve_pairs(5);
    assert(obj.capacity() >= 10);

    obj.set_string("name", "Ada");
    obj.clear();

    assert(obj.empty());
    assert(obj.raw_size() == 0);
    assert(obj.size_pairs() == 0);
  }

  return 0;
}
