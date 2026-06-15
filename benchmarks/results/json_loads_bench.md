# json_loads_bench results

Command:

```bash
./build-ninja/bin/json_loads_bench
```

Build mode:

```txt
dev, unoptimized + debuginfo
```

## Results

| Benchmark                               | Iterations |       Total |       Average |
| --------------------------------------- | ---------: | ----------: | ------------: |
| `loads() small object`                  |     300000 |  2932730 us | 9.77577 us/op |
| `try_loads() small object`              |     300000 |  2948334 us | 9.82778 us/op |
| `loads() medium object`                 |     100000 |  4315692 us | 43.1569 us/op |
| `try_loads() medium object`             |     100000 |  4731958 us | 47.3196 us/op |
| `loads() large object`                  |      10000 | 34225861 us | 3422.59 us/op |
| `try_loads() large object`              |      10000 | 33257413 us | 3325.74 us/op |
| `manual nlohmann parse small baseline`  |     300000 |  3400230 us | 11.3341 us/op |
| `manual nlohmann parse medium baseline` |     100000 |  4919326 us | 49.1933 us/op |
| `try_loads() invalid json`              |     100000 |  1338151 us | 13.3815 us/op |
| `load_file() small file`                |       3000 |    66419 us | 22.1397 us/op |
| `try_load_file() small file`            |       3000 |    66149 us | 22.0497 us/op |
| `load_file() medium file`               |       3000 |   188766 us |  62.922 us/op |
| `try_load_file() medium file`           |       3000 |   187155 us |  62.385 us/op |
| `load_file() large file`                |       3000 | 10524759 us | 3508.25 us/op |
| `try_load_file() large file`            |       3000 | 10557531 us | 3519.18 us/op |
| `try_load_file() invalid file`          |       3000 |    74050 us | 24.6833 us/op |

Checksum:

```txt
60669000
```

## Quick interpretation

`loads()` and `try_loads()` are very close for valid small JSON input. The safe wrapper adds almost no visible overhead in this dev build for successful parsing.

Medium and large JSON parsing are dominated by the underlying `nlohmann::json::parse()` cost.

`try_loads() invalid json` is slower than successful small parsing because it catches a parse exception, but it remains acceptable for safe external input handling.

File loading adds filesystem I/O on top of parsing. Small files are around 22 us/op in this run, while large files are around 3.5 ms/op.

`try_load_file()` is close to `load_file()` for valid files because the safe wrapper only catches errors; it does not change the core read/parse path.
