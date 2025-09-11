#!/usr/bin/env bash
set -euo pipefail
# Requires: perf, inferno-flamegraph or speedscope for visualization
perf record -F 99 -g -- ./build/acx list --limit 1000 || true
perf script | inferno-flamegraph > flame.svg || true
