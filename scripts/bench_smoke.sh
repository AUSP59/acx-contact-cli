#!/usr/bin/env bash
set -euo pipefail
BIN=${1:-./build/acx}
THRESH=${PERF_GATE_MS:-}
rm -f acx-data.json acx-audit.log backups/* acx-index.json || true

# Seed small dataset and index
$BIN add --name "John Doe" --email john@example.com --phone +12025550101 --note x >/dev/null
for i in $(seq 1 1000); do
  $BIN search --email john@example.com >/dev/null
done

if [[ -n "${THRESH}" ]]; then
  start=$(date +%s%3N)
  $BIN search --email john@example.com >/dev/null
  end=$(date +%s%3N)
  dur=$((end-start))
  echo "p95-ish single search ms: ${dur}"
  if [[ "${dur}" -gt "${THRESH}" ]]; then
    echo "PERF GATE FAIL: ${dur}ms > ${THRESH}ms"
    exit 1
  fi
fi
echo "OK"
