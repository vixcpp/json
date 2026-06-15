# json_simple_bench results

Command:

```bash
./build-ninja/bin/json_simple_bench
```

Build mode:

```txt
dev, unoptimized + debuginfo
```

## Results

| Benchmark                             | Iterations |      Total |        Average |
| ------------------------------------- | ---------: | ---------: | -------------: |
| `token construct int`                 |    1000000 |  185720 us |  0.18572 us/op |
| `token construct string`              |    1000000 |  725366 us | 0.725366 us/op |
| `token setters`                       |    1000000 | 1017312 us |  1.01731 us/op |
| `array_t push_int`                    |    1000000 | 1805128 us |  1.80513 us/op |
| `array_t push mixed values`           |     500000 | 1986627 us |  3.97325 us/op |
| `array_t ensure grows`                |     500000 | 1442119 us |  2.88424 us/op |
| `array_t erase_at middle`             |     500000 | 1612177 us |  3.22435 us/op |
| `array_t build 100 integers`          |     100000 | 1669829 us |  16.6983 us/op |
| `kvs set new keys`                    |     500000 | 3978498 us |    7.957 us/op |
| `kvs replace existing key`            |     500000 |  985299 us |   1.9706 us/op |
| `kvs operator[] existing and missing` |     500000 | 2247624 us |  4.49525 us/op |
| `kvs contains existing key`           |    1000000 | 5211770 us |  5.21177 us/op |
| `kvs contains missing key`            |    1000000 | 5365780 us |  5.36578 us/op |
| `kvs typed getters`                   |    1000000 | 6673194 us |  6.67319 us/op |
| `kvs erase existing key`              |     500000 | 2853816 us |  5.70763 us/op |
| `kvs erase_if`                        |     500000 | 3431958 us |  6.86392 us/op |
| `kvs for_each_pair`                   |    1000000 | 5148123 us |  5.14812 us/op |
| `kvs keys()`                          |     500000 | 2867570 us |  5.73514 us/op |
| `kvs merge_from overwrite=true`       |     500000 | 6575710 us |  13.1514 us/op |
| `kvs merge_from overwrite=false`      |     500000 | 8915278 us |  17.8306 us/op |
| `token ensure_array`                  |     500000 |  891599 us |   1.7832 us/op |
| `token ensure_object`                 |     500000 | 2022182 us |  4.04436 us/op |
| `nested Simple object build`          |     100000 | 1723666 us |  17.2367 us/op |
| `manual vector<token> array baseline` |     100000 | 2581746 us |  25.8175 us/op |

Checksum:

```txt
3150174388890
```

## Quick interpretation

Primitive `token` construction is very cheap. Integer token construction is under 0.2 us/op in this dev build.

String token construction is more expensive than integer construction because it allocates or copies string storage.

`array_t build 100 integers` is faster than the manual `vector<token>` baseline in this run because the helper reserves capacity and uses the module’s array abstraction directly.

`kvs` operations are naturally more expensive than array operations because `kvs` stores object pairs in a flat vector and performs linear key lookup.

Replacing an existing key is much cheaper than inserting several new keys.

`merge_from overwrite=false` is slower than `overwrite=true` here because it checks whether each incoming key already exists before setting it.

`nested Simple object build` remains reasonable for internal structured payload construction and avoids depending on `nlohmann::json` directly.
