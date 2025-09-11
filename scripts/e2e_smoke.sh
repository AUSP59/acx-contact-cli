#!/usr/bin/env bash
set -euo pipefail
b=build
cmake -S . -B $b -DCMAKE_BUILD_TYPE=Release
cmake --build $b --target acx
$b/acx --json add --name "Test User" --email test@ex.org --phone +525511112222 >/dev/null
$b/acx --json list | grep '"status":"ok"'
$b/acx --json search --email test@ex.org | grep '"results"'
$b/acx --json audit | grep '"status":"ok"'
echo "E2E smoke OK"
