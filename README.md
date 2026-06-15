# Vix.cpp JSON Module

JSON utilities for Vix.cpp applications.

The JSON module provides a small, practical layer on top of `nlohmann::json`. It is designed to keep common JSON work simple while staying fully compatible with the underlying `nlohmann::json` API.

It provides:

- JSON construction helpers,
- JSON parsing helpers,
- JSON serialization helpers,
- safe access and conversion helpers,
- a small JPath syntax for dynamic nested access,
- a lightweight internal Simple value model,
- examples,
- tests,
- benchmarks.

## What this module is for

Use this module when you need to:

- build JSON API responses,
- parse JSON text,
- read and write JSON files,
- safely access user-provided JSON data,
- mutate nested JSON values with dynamic paths,
- create small internal structured payloads without exposing `nlohmann::json` everywhere.

The module is header-only and exposes the public target:

```txt
vix::json
```

## Main include

For most users, include:

```cpp
#include <vix/json.hpp>
```

or:

```cpp
#include <vix/json/json.hpp>
```

Then use:

```cpp
using namespace vix::json;
```

## Minimal example

```cpp
#include <vix/json.hpp>

#include <iostream>

int main()
{
  using namespace vix::json;

  Json user = o(
      "id", 1,
      "name", "Ada",
      "skills", a("C++", "JSON", "Vix.cpp"));

  jset(user, "profile.city", "Kampala");

  if (const Json *name = jget(user, "name"))
    std::cout << "name: " << name->get<std::string>() << "\n";

  std::cout << dumps_pretty(user) << "\n";

  return 0;
}
```

## Public headers

```txt
include/vix/json/
  build.hpp
  convert.hpp
  dumps.hpp
  jpath.hpp
  json.hpp
  loads.hpp
  Simple.hpp
```

## JSON types

The module exposes two main JSON aliases:

```cpp
using Json = nlohmann::json;
using OrderedJson = nlohmann::ordered_json;
```

`Json` is the default type.

`OrderedJson` is used by the object builder `o()` to preserve insertion order and produce deterministic output.

## Builders

Header:

```cpp
#include <vix/json/build.hpp>
```

### Object builder

```cpp
auto user = o(
    "id", 42,
    "name", "Gaspard",
    "active", true);
```

`o()` returns `OrderedJson`.

The number of arguments must be even:

```cpp
auto ok = o("name", "Vix.cpp", "version", "1.0.0");
```

This is invalid and should not compile:

```cpp
auto bad = o("name", "Vix.cpp", "missing_value");
```

### Array builder

```cpp
Json values = a(1, 2, 3, "hello", true);
```

### Key/value builder

```cpp
Json config = kv({
    {"name", Json("Vix.cpp")},
    {"debug", Json(true)},
    {"port", Json(8080)},
});
```

`kv()` is useful when pairs are created dynamically. For normal static code, prefer `o()`.

## Parsing JSON

Header:

```cpp
#include <vix/json/loads.hpp>
```

### Throwing parse

```cpp
Json j = loads(R"({"id": 1, "name": "Ada"})");
```

`loads()` throws if the input is invalid.

### Safe parse

```cpp
auto maybe = try_loads("{bad json");

if (!maybe)
{
  // invalid JSON
}
```

`try_loads()` never throws. It returns `std::nullopt` on failure.

## Loading JSON files

```cpp
Json config = load_file("config.json");
```

`load_file()` throws if:

- the file cannot be opened,
- the file is empty,
- the content is invalid JSON.

Safe version:

```cpp
auto config = try_load_file("config.json");

if (!config)
{
  // missing file, empty file, or invalid JSON
}
```

## Dumping JSON

Header:

```cpp
#include <vix/json/dumps.hpp>
```

### Pretty JSON

```cpp
std::string text = dumps(j);
std::string pretty = dumps_pretty(j);
```

Default indentation is 2 spaces.

Custom indentation:

```cpp
std::string text = dumps(j, 4);
```

### Compact JSON

```cpp
std::string text = dumps_compact(j);
```

### ASCII escaping

```cpp
std::string text = dumps_compact(j, true);
```

When `ensure_ascii` is `true`, non-ASCII characters are escaped.

## Writing JSON files

```cpp
dump_file("out/config.json", j);
```

`dump_file()` writes through a temporary file and then replaces the target file. It also creates parent directories when needed.

This reduces the chance of leaving a partially written JSON file.

## Safe access and conversion

Header:

```cpp
#include <vix/json/convert.hpp>
```

### Pointer access

```cpp
const Json *name = ptr(j, "name");

if (name)
{
  // key exists
}
```

Array access:

```cpp
const Json *first = ptr(arr, 0);
```

`ptr()` returns `nullptr` when the value is missing or the shape is invalid.

### Optional conversion

```cpp
auto id = get_opt<int>(j, "id");

if (id)
{
  // valid integer
}
```

### Default value conversion

```cpp
int port = get_or<int>(j, "port", 8080);
```

### Strict conversion

```cpp
int id = ensure<int>(j, "id");
```

`ensure()` throws when the value is missing or has the wrong type.

Recommended usage:

```txt
External input:       get_opt() or get_or()
Trusted internal data: ensure()
Low-level access:     ptr()
```

## JPath

Header:

```cpp
#include <vix/json/jpath.hpp>
```

JPath is a small path syntax for navigating and mutating JSON values.

It supports:

```txt
user.name
settings.theme
users[0].email
roles[1]
["complex.key"].value
["a b c"][0]
```

It is not a full JSONPath implementation. It is intentionally small and focused on common application use cases.

### Read nested values

```cpp
Json j = loads(R"({
  "user": {
    "name": "Ada",
    "roles": ["admin", "editor"]
  }
})");

if (const Json *role = jget(j, "user.roles[1]"))
{
  std::cout << role->get<std::string>() << "\n";
}
```

The const version:

```cpp
const Json *jget(const Json &, path)
```

does not throw for invalid or missing paths. It returns `nullptr`.

### Write nested values

```cpp
Json j = Json::object();

jset(j, "user.profile.name", "Gaspard");
jset(j, "user.roles[0]", "admin");
```

`jset()` creates missing intermediate objects and arrays.

Result:

```json
{
  "user": {
    "profile": {
      "name": "Gaspard"
    },
    "roles": ["admin"]
  }
}
```

### Mutable access

```cpp
Json j = Json::object();

Json *name = jget(j, "user.name");
*name = "Ada";
```

The mutable version can create missing nodes and may throw on invalid syntax.

## Simple value model

Header:

```cpp
#include <vix/json/Simple.hpp>
```

`Simple.hpp` provides a lightweight JSON-like model used for internal Vix APIs.

It does not depend on `nlohmann::json`.

It supports:

- null,
- bool,
- int64,
- double,
- string,
- array,
- object.

Main types:

```cpp
token
array_t
kvs
```

### Basic values

```cpp
token a = nullptr;
token b = true;
token c = 42;
token d = 3.14;
token e = "hello";
```

### Arrays

```cpp
array_t values = simple_array({
    1,
    "two",
    true,
    nullptr,
});
```

Mutation:

```cpp
values.push_int(42);
values.push_string("Vix.cpp");
values.ensure(10) = "created";
```

### Objects

```cpp
kvs user = simple_obj({
    "name", "Ada",
    "age", 30,
    "active", true,
});
```

Typed access:

```cpp
std::string name = user.get_string_or("name", "unknown");
std::int64_t age = user.get_i64_or("age", 0);
bool active = user.get_bool_or("active", false);
```

Mutation:

```cpp
user.set_string("city", "Kampala");
user.set_bool("verified", true);
```

### Nested Simple payloads

```cpp
kvs root;

kvs &user = root.ensure_object("user");
user.set_string("name", "Ada");

array_t &skills = user.ensure_array("skills");
skills.push_string("C++");
skills.push_string("JSON");
```

### Convert Simple to Json

Header:

```cpp
#include <vix/json/convert.hpp>
```

```cpp
kvs user = simple_obj({
    "name", "Ada",
    "age", 30,
});

Json j = simple_to_json(user);
```

Aliases:

```cpp
Json j1 = to_json(token("hello"));
Json j2 = to_json(simple_array({1, 2, 3}));
Json j3 = to_json(simple_obj({"name", "Ada"}));
```

## Examples

Examples are located in:

```txt
examples/
```

Main examples:

```txt
quick_start.cpp
builders.cpp
jpath.cpp
io.cpp
```

Simple examples:

```txt
examples/json_simple/
  01_basic_values.cpp
  02_arrays.cpp
  03_objects.cpp
  04_nested.cpp
  05_mutation.cpp
  06_iteration.cpp
  07_merge_and_erase.cpp
  08_to_nlohmann_json.cpp
  bench_simple_vs_nlohmann.cpp
```

Build examples:

```bash
vix build --build-target all -v -- -DVIX_JSON_BUILD_EXAMPLES=ON
```

Run examples:

```bash
./build-ninja/bin/vix_json_quick
./build-ninja/bin/vix_json_builders
./build-ninja/bin/vix_json_jpath
./build-ninja/bin/vix_json_io
./build-ninja/bin/vix_json_simple_01_basic_values
./build-ninja/bin/vix_json_simple_08_to_nlohmann_json
```

## Tests

Tests are located in:

```txt
tests/
```

Current test files:

```txt
json_build_test.cpp
json_convert_test.cpp
json_dumps_test.cpp
json_include_test.cpp
json_jpath_test.cpp
json_loads_test.cpp
json_simple_test.cpp
```

Build and run tests:

```bash
vix build --build-target all -v -- -DVIX_JSON_BUILD_TESTS=ON
vix tests
```

Or build everything:

```bash
vix build --build-target all -v -- \
  -DVIX_JSON_BUILD_EXAMPLES=ON \
  -DVIX_JSON_BUILD_TESTS=ON \
  -DVIX_JSON_BUILD_BENCHMARKS=ON

vix tests
```

## Benchmarks

Benchmarks are located in:

```txt
benchmarks/
```

Benchmark files:

```txt
json_build_bench.cpp
json_convert_bench.cpp
json_dumps_bench.cpp
json_jpath_bench.cpp
json_loads_bench.cpp
json_simple_bench.cpp
```

Build benchmarks:

```bash
vix build --build-target all -v -- -DVIX_JSON_BUILD_BENCHMARKS=ON
```

Run benchmarks:

```bash
./build-ninja/bin/json_build_bench
./build-ninja/bin/json_convert_bench
./build-ninja/bin/json_dumps_bench
./build-ninja/bin/json_jpath_bench
./build-ninja/bin/json_loads_bench
./build-ninja/bin/json_simple_bench
```

Benchmark result snapshots are stored in:

```txt
benchmarks/results/
```

Current benchmark notes:

```txt
benchmarks/README.md
benchmarks/results/json_build_bench.md
benchmarks/results/json_convert_bench.md
benchmarks/results/json_dumps_bench.md
benchmarks/results/json_jpath_bench.md
benchmarks/results/json_loads_bench.md
benchmarks/results/json_simple_bench.md
benchmarks/results/vix_json_simple_bench_simple_vs_nlohmann.md
```

## CMake options

| Option                      | Default | Description           |
| --------------------------- | ------: | --------------------- |
| `VIX_JSON_BUILD_EXAMPLES`   |   `OFF` | Build JSON examples   |
| `VIX_JSON_BUILD_TESTS`      |   `OFF` | Build JSON tests      |
| `VIX_JSON_BUILD_BENCHMARKS` |   `OFF` | Build JSON benchmarks |

Example:

```bash
vix build --build-target all -v -- \
  -DVIX_JSON_BUILD_EXAMPLES=ON \
  -DVIX_JSON_BUILD_TESTS=ON \
  -DVIX_JSON_BUILD_BENCHMARKS=ON
```

## Dependency provider

The module uses `nlohmann_json`.

Provider resolution:

1. use installed `nlohmann_json` CMake config package,
2. use CMake find-module mode,
3. fetch a header-only fallback for local builds.

Public target:

```txt
vix::json
```

The module itself is header-only.

## Installation

Standalone installation installs the headers and exports the package target.

Typical standalone install flow:

```bash
vix build --build-target all -v
```

For packaging, the CMake install rules export:

```txt
vix_jsonTargets
```

When built as part of the Vix umbrella, it exports into:

```txt
VixTargets
```

## API guide

### For JSON construction

Use:

```cpp
o("key", value)
a(value1, value2)
kv({{"key", Json(value)}})
```

### For parsing

Use:

```cpp
loads(text)
try_loads(text)
load_file(path)
try_load_file(path)
```

### For dumping

Use:

```cpp
dumps(j)
dumps_pretty(j)
dumps_compact(j)
dump_file(path, j)
```

### For safe access

Use:

```cpp
ptr(j, key)
ptr(arr, index)
get_opt<T>(j)
get_or<T>(j, fallback)
ensure<T>(j)
```

### For dynamic paths

Use:

```cpp
jget(j, "user.name")
jset(j, "user.name", "Ada")
```

### For lightweight internal payloads

Use:

```cpp
token
array_t
kvs
simple_array(...)
simple_obj(...)
```

## Design notes

The module keeps `nlohmann::json` compatibility instead of hiding it behind a new large abstraction.

The helpers are intentionally small:

- `build.hpp` improves construction readability,
- `loads.hpp` makes parsing explicit,
- `dumps.hpp` makes serialization and file output explicit,
- `convert.hpp` reduces repetitive safe-access boilerplate,
- `jpath.hpp` handles dynamic nested paths,
- `Simple.hpp` provides a minimal internal payload model.

The goal is not to replace `nlohmann::json`. The goal is to make JSON work easier and more consistent inside Vix.cpp applications.

## Useful links

- JSON documentation: https://docs.vixcpp.com/modules/json/
- JSON API reference: https://docs.vixcpp.com/modules/json/api-reference
- Build command: https://docs.vixcpp.com/cli/build
- Tests command: https://docs.vixcpp.com/cli/tests
- Documentation: https://docs.vixcpp.com/
- Engineering notes: https://blog.vixcpp.com/
- Registry: https://registry.vixcpp.com/
- GitHub: https://github.com/vixcpp/vix
