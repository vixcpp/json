# json_jpath_bench results

Command:

```bash
./build-ninja/bin/json_jpath_bench
```

Build mode:

```txt
dev, unoptimized + debuginfo
```

## Results

| Benchmark                                | Iterations |       Total |        Average |
| ---------------------------------------- | ---------: | ----------: | -------------: |
| `tokenize_path() dot path`               |     500000 |   868909 us |  1.73782 us/op |
| `tokenize_path() array path`             |     500000 |   747875 us |  1.49575 us/op |
| `tokenize_path() quoted key path`        |     500000 |   804859 us |  1.60972 us/op |
| `jget const simple dot path`             |    1000000 |  2812652 us |  2.81265 us/op |
| `jget const nested number path`          |    1000000 |  3093014 us |  3.09301 us/op |
| `jget const array path`                  |    1000000 |  2369436 us |  2.36944 us/op |
| `jget const quoted key path`             |     500000 |  1170752 us |   2.3415 us/op |
| `jget const quoted key with spaces path` |     500000 |   829064 us |  1.65813 us/op |
| `jget const missing key`                 |    1000000 |  2722665 us |  2.72267 us/op |
| `jget const out-of-range index`          |    1000000 |  1863334 us |  1.86333 us/op |
| `manual object access baseline`          |    1000000 |   949552 us | 0.949552 us/op |
| `manual array access baseline`           |    1000000 |   722939 us | 0.722939 us/op |
| `jset existing simple path`              |     100000 | 32003767 us |  320.038 us/op |
| `jset existing array path`               |     100000 | 32904915 us |  329.049 us/op |
| `jset create nested object path`         |     100000 |  1331789 us |  13.3179 us/op |
| `jset create array path`                 |     100000 |  1681094 us |  16.8109 us/op |
| `jset quoted key path`                   |     100000 |   926111 us |  9.26111 us/op |
| `mutable jget create nested object path` |     100000 |   960076 us |  9.60076 us/op |
| `mutable jget create array path`         |     100000 |  1640400 us |   16.404 us/op |
| `jset invalid path`                      |     100000 |   658152 us |  6.58152 us/op |

Checksum:

```txt
28142427780
```

## Quick interpretation

`tokenize_path()` stays around 1.5 to 1.8 us/op in this dev build.

`jget const` is slower than direct manual nlohmann access because it parses the path on each call, validates every segment, and supports object keys, array indices, and quoted keys.

Manual access is faster when the path is known at compile time, but `jget()` is useful when the path is dynamic.

`jset existing simple path` and `jset existing array path` are much slower in this run because each iteration copies the full sample JSON before writing. The measured cost includes the copy of the existing root object, not only the `jset()` operation.

Creating a fresh nested object or array is much cheaper here because the starting JSON value is empty.
