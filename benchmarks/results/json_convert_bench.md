# json_convert_bench results

Command:

```bash
./build-ninja/bin/json_convert_bench
```

Build mode:

```txt
dev, unoptimized + debuginfo
```

## Results

| Benchmark                            | Iterations |      Total |        Average |
| ------------------------------------ | ---------: | ---------: | -------------: |
| `ptr(obj, key) existing`             |    1000000 |  417831 us | 0.417831 us/op |
| `ptr(obj, key) missing`              |    1000000 |  385048 us | 0.385048 us/op |
| `ptr(array, index) existing`         |    1000000 |   40997 us | 0.040997 us/op |
| `ptr(array, index) out of range`     |    1000000 |   11701 us | 0.011701 us/op |
| `get_opt<int>(json)`                 |    1000000 |  353392 us | 0.353392 us/op |
| `get_opt<string>(json)`              |     500000 |  307837 us | 0.615674 us/op |
| `get_opt<int>(obj, key)`             |    1000000 |  475619 us | 0.475619 us/op |
| `get_opt<int>(obj, missing)`         |    1000000 |  406114 us | 0.406114 us/op |
| `get_opt<int>(array, index)`         |    1000000 |   84544 us | 0.084544 us/op |
| `get_opt<int>(array, missing)`       |    1000000 |   33148 us | 0.033148 us/op |
| `get_or<int>(json, default) valid`   |    1000000 |  361693 us | 0.361693 us/op |
| `get_or<int>(json, default) invalid` |    1000000 | 4140048 us |  4.14005 us/op |
| `get_or<int>(obj, key, default)`     |    1000000 |  482386 us | 0.482386 us/op |
| `get_or<int>(array, index, default)` |    1000000 |   89959 us | 0.089959 us/op |
| `ensure<int>(json)`                  |    1000000 |  319199 us | 0.319199 us/op |
| `ensure<int>(obj, key)`              |    1000000 |  434056 us | 0.434056 us/op |
| `simple_to_json(token primitives)`   |     500000 |  646244 us |  1.29249 us/op |
| `simple_to_json(array_t)`            |     100000 |  583056 us |  5.83056 us/op |
| `simple_to_json(kvs)`                |     100000 | 1694523 us |  16.9452 us/op |
| `manual object access baseline`      |    1000000 |  804238 us | 0.804238 us/op |
| `manual array access baseline`       |    1000000 |   54138 us | 0.054138 us/op |

Checksum:

```txt
159543350000
```

## Quick interpretation

Array pointer access is very cheap because it only checks the array type and index bounds.

Object key lookup is more expensive than array access because it searches by key.

`get_or<int>(json, default) invalid` is much slower because the invalid conversion path uses exception handling internally through `get_opt`.

`ensure<int>(json)` is the fastest strict conversion path in this benchmark because it does not wrap the conversion into an optional.

`simple_to_json(kvs)` is naturally more expensive than primitive and array conversion because it walks object pairs and recursively converts nested Simple values.
