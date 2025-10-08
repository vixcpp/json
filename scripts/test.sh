# scripts/test.sh
#!/usr/bin/env bash
set -e
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j
