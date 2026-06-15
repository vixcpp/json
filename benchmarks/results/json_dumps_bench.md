# json_dumps_bench results

Command:

```bash
./build-ninja/bin/json_dumps_bench
```

Build mode:

```txt
dev, unoptimized + debuginfo
```

## Results

| Benchmark                               | Iterations |      Total |       Average |
| --------------------------------------- | ---------: | ---------: | ------------: |
| `dumps() small pretty`                  |     200000 |  560650 us | 2.80325 us/op |
| `dumps_compact() small`                 |     200000 |  479024 us | 2.39512 us/op |
| `dumps() medium pretty`                 |     100000 | 1006242 us | 10.0624 us/op |
| `dumps_compact() medium`                |     100000 |  843296 us | 8.43296 us/op |
| `dumps_pretty() medium`                 |     100000 |  995703 us | 9.95703 us/op |
| `dumps() large pretty`                  |      10000 | 4912355 us | 491.236 us/op |
| `dumps_compact() large`                 |      10000 | 4009584 us | 400.958 us/op |
| `dumps_compact() ensure_ascii=false`    |     100000 | 1014341 us | 10.1434 us/op |
| `dumps_compact() ensure_ascii=true`     |     100000 | 1027169 us | 10.2717 us/op |
| `manual nlohmann dump compact baseline` |     100000 |  910108 us | 9.10108 us/op |
| `manual nlohmann dump pretty baseline`  |     100000 | 1120320 us | 11.2032 us/op |
| `dump_file() medium`                    |       1000 |   81501 us |  81.501 us/op |
| `dump_file() large`                     |       1000 | 1193260 us | 1193.26 us/op |

Checksum:

```txt
418486780
```

## Quick interpretation

Compact dumping is faster than pretty dumping because it writes less whitespace.

Large JSON serialization is naturally much more expensive because output size dominates the operation.

`dumps_pretty()` is effectively the explicit alias of `dumps()`, and its result is close to the `dumps() medium pretty` measurement.

`ensure_ascii=true` is slightly slower than `ensure_ascii=false` in this run because non-ASCII escaping adds extra work.

`dump_file()` includes serialization, filesystem writes, temp file creation, and rename/copy behavior. It should be interpreted as an I/O benchmark, not only a JSON serialization benchmark.
