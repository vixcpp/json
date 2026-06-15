# json_build_bench results

Command:

```bash
./build-ninja/bin/json_build_bench
```

Build mode:

```txt
dev, unoptimized + debuginfo
```

## Results

| Benchmark                         | Iterations |      Total |       Average |
| --------------------------------- | ---------: | ---------: | ------------: |
| `json::o small object`            |     200000 |  783352 us | 3.91676 us/op |
| `json::o medium object`           |     100000 |  994598 us | 9.94598 us/op |
| `json::a small array`             |     200000 |  618123 us | 3.09062 us/op |
| `json::a medium array`            |     100000 |  569159 us | 5.69159 us/op |
| `json::o nested object and array` |     100000 | 2179145 us | 21.7915 us/op |
| `json::kv object`                 |     100000 |  660333 us | 6.60333 us/op |
| `manual nlohmann object baseline` |     200000 |  675274 us | 3.37637 us/op |
| `manual nlohmann array baseline`  |     200000 |  603633 us | 3.01817 us/op |
| `large generated array`           |      20000 |  684229 us | 34.2114 us/op |

Checksum:

```txt
100409560000
```

## Quick interpretation

`json::a small array` is close to the manual nlohmann array baseline.

`json::o small object` is slightly slower than manual object construction, which is expected because it provides a cleaner helper API and uses ordered JSON construction.

`json::o nested object and array` is the most expensive helper case here because it creates multiple nested objects and arrays in one expression.

`json::kv object` is faster than the medium `json::o` object case in this run, but it requires explicit `Json(...)` values and is less ergonomic for normal application code.
