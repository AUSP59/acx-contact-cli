#!/usr/bin/env bash
set -euo pipefail
BIN=${1:-./build/acx}
DATA=bench-data.jsonl
rm -f acx-data.json acx-audit.log backups/* || true
$BIN add --name "John Doe" --email john@example.com --phone +12025550101 --note x >/dev/null
$BIN add --name "Jane Roe" --email jane@example.com --phone +12025550102 --note x >/dev/null
for i in $(seq 1 1000); do
  $BIN search --email john@example.com >/dev/null
done
echo "OK"
