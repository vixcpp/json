# vix_json_simple_bench_simple_vs_nlohmann results

Command:

```bash
./build-ninja/bin/vix_json_simple_bench_simple_vs_nlohmann
```

Build mode:

```txt
dev, unoptimized + debuginfo
```

## Warm-up / single run

| Benchmark                                   |    Total |
| ------------------------------------------- | -------: |
| `build Simple`                              |  7272 us |
| `build nlohmann::json`                      |  5852 us |
| `convert Simple -> Json`                    | 11896 us |
| `dump from Simple (Simple -> Json -> dump)` |  6440 us |
| `dump from nlohmann::json (build -> dump)`  |  3422 us |
| `dump only (prebuilt nlohmann::json)`       |  1018 us |

## Bench run

Configuration:

```txt
iters=2000
items=64
```

| Benchmark                                   |     Total |
| ------------------------------------------- | --------: |
| `build Simple`                              | 216055 us |
| `build nlohmann::json`                      | 199027 us |
| `convert Simple -> Json`                    | 466155 us |
| `dump from Simple (Simple -> Json -> dump)` | 482676 us |
| `dump from nlohmann::json (build -> dump)`  | 285219 us |
| `dump only (prebuilt nlohmann::json)`       |  86580 us |

Sink:

```txt
sink_u64=0x72e2ebeacd51771a
sink_sz=0x3972dc0ce1fb22a0
```

## Quick interpretation

In this dev build, direct `nlohmann::json` construction is faster than the Simple model for this benchmark shape.

The largest cost for Simple appears when converting `Simple -> Json`. That is expected because the conversion walks the Simple structure and creates a full `nlohmann::json` tree.

Dumping from Simple is slower than dumping from an already-built `nlohmann::json` value because it includes conversion plus serialization.

The Simple model is still useful for lightweight internal payloads where modules need a small JSON-like value model without depending directly on `nlohmann::json`. For final JSON text output, direct `nlohmann::json` remains the faster path in this benchmark.
