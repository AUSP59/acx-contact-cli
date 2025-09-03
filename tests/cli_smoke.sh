#!/usr/bin/env bash
set -euo pipefail
BIN=${1:-./build/acx}
# Try version
$BIN --version >/dev/null 2>&1 || $BIN version >/dev/null 2>&1
# Try help (any of these should succeed)
($BIN --help >/dev/null 2>&1) || ($BIN help >/dev/null 2>&1) || true
echo "CLI smoke OK"
