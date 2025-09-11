#!/usr/bin/env bash
set -euo pipefail
rm -rf build1 build2
export SOURCE_DATE_EPOCH=1700000000
cmake -S . -B build1 -DCMAKE_BUILD_TYPE=Release
cmake --build build1 --config Release
cmake -S . -B build2 -DCMAKE_BUILD_TYPE=Release
cmake --build build2 --config Release
sha256sum build1/acx build2/acx || true
