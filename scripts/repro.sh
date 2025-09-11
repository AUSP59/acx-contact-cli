#!/usr/bin/env bash
set -euo pipefail
export SOURCE_DATE_EPOCH=1700000000
CFLAGS="-ffile-prefix-map=$PWD=." CXXFLAGS="-ffile-prefix-map=$PWD=." cmake -S . -B build-repro -DCMAKE_BUILD_TYPE=Release
cmake --build build-repro --config Release
strip build-repro/acx || true
