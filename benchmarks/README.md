# JSON benchmarks

This directory contains standalone benchmarks for the Vix.cpp JSON module.

The benchmarks are intentionally simple and dependency-free. They use `std::chrono` and print:

- total time in microseconds,
- number of iterations,
- average time per operation,
- checksum to prevent the compiler from removing the measured work.

These benchmarks are meant for quick local checks, regression tracking, and understanding the relative cost of each JSON helper.

## Build

From the JSON module root:

```bash
vix build --build-target all -v -- \
  -DVIX_JSON_BUILD_EXAMPLES=ON \
  -DVIX_JSON_BUILD_TESTS=ON \
  -DVIX_JSON_BUILD_BENCHMARKS=ON
```

## Run

```bash
./build-ninja/bin/json_build_bench
./build-ninja/bin/json_convert_bench
./build-ninja/bin/json_dumps_bench
./build-ninja/bin/json_jpath_bench
./build-ninja/bin/json_loads_bench
./build-ninja/bin/json_simple_bench
./build-ninja/bin/vix_json_simple_bench_simple_vs_nlohmann
```

## Result snapshots

Benchmark result snapshots are stored in:

```txt
benchmarks/results/
```

Current result files:

```txt
json_build_bench.md
json_convert_bench.md
json_dumps_bench.md
json_jpath_bench.md
json_loads_bench.md
json_simple_bench.md
vix_json_simple_bench_simple_vs_nlohmann.md
```

## Summary

The current benchmark run was executed in:

```txt
dev, unoptimized + debuginfo
```

So the numbers should not be treated as final release performance numbers. They are useful mainly for comparing helpers against each other and for detecting regressions over time.

## Main findings

### Builders

`json::a()` is close to manual `nlohmann::json` array construction for small arrays.

`json::o()` is slightly slower than manual object construction, which is expected because it provides a more ergonomic builder API and uses ordered JSON construction for deterministic output.

Nested builders are more expensive because they allocate and construct several JSON objects and arrays in one expression.

For most application code, the builder cost is acceptable because these helpers are designed for readability, API responses, tests, fixtures, and configuration generation.

### Conversions and accessors

Array access is very cheap because it only needs an array check and an index bounds check.

Object key access is more expensive because it requires key lookup.

`get_opt()` and `get_or()` are convenient and fast on valid values, but invalid type conversion can be much slower because the underlying `nlohmann::json` conversion path uses exceptions.

`ensure()` is the fastest strict conversion path in the successful case, but it intentionally throws when the value is missing or invalid.

Use:

```txt
get_opt() / get_or()  for external or uncertain input
ensure()              for trusted internal data
ptr()                 for low-level optional access
```

### JPath

`jget()` is slower than direct manual `nlohmann::json` access because it parses the path string and validates each segment on every call.

Manual access is faster when the path is known at compile time:

```cpp
j["user"]["profile"]["name"]
```

But `jget()` is useful when the path is dynamic:

```cpp
jget(j, "user.profile.name")
```

`jset()` is useful for dynamic mutation and can create missing objects or arrays automatically.

Some `jset()` benchmarks that write into an existing large sample JSON are expensive because each iteration copies the full JSON value before writing. Those numbers measure both JSON copy cost and JPath mutation cost.

### Dumps

Compact dumping is faster than pretty dumping because it writes less whitespace.

Large JSON dumping is naturally much more expensive because output size dominates the operation.

`dumps_pretty()` behaves like an explicit alias of `dumps()`.

`dump_file()` measures more than JSON serialization. It includes serialization, file writing, temporary file handling, and replacement behavior. It should be read as an I/O benchmark.

### Loads

`loads()` and `try_loads()` are close for valid JSON input.

For valid data, the safe wrapper adds little overhead because it only catches errors when they occur.

Invalid parsing is slower because it goes through exception handling.

File loading adds filesystem I/O on top of JSON parsing. Small files remain cheap, while large files are dominated by parsing and file read cost.

### Simple model

The Simple model is lightweight for internal structured payloads.

Primitive `token` construction is cheap. Integer token construction is especially cheap.

String token construction costs more because string storage must be allocated or copied.

`array_t` is efficient for array-like payloads.

`kvs` operations are more expensive than array operations because objects are stored as a flat key/value vector and key lookup is linear.

`Simple -> Json` conversion is one of the most expensive Simple operations because it walks the Simple tree and creates a full `nlohmann::json` tree.

Direct `nlohmann::json` is faster when the final goal is immediate JSON serialization. Simple remains useful when the goal is a small JSON-like internal data model without exposing `nlohmann::json` everywhere.

## Practical conclusions

Use `Json` / `nlohmann::json` directly when the main task is parsing, serializing, or heavy JSON processing.

Use `json::o()` and `json::a()` when readability matters and the JSON value is small or medium-sized.

Use `ptr()`, `get_opt()`, and `get_or()` for safe access to external input.

Use `ensure()` for trusted internal data where failure should be loud.

Use `jget()` and `jset()` when paths are dynamic or when automatic creation of nested structures is useful.

Use the Simple model for internal module payloads and lightweight structured data, not as a faster replacement for `nlohmann::json` serialization.

## Important note

These benchmarks are not a replacement for a full benchmarking framework.

They are intentionally simple and should be interpreted as local development measurements. Release builds, CPU differences, compiler flags, and system load can change the numbers significantly.
