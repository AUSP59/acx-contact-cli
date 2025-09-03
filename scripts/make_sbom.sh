#!/usr/bin/env bash
set -euo pipefail
if command -v cyclonedx-generate >/dev/null 2>&1; then
  cyclonedx-generate -o compliance/SBOM.cdx.json
else
  echo "cyclonedx-generate not found; using static stub at compliance/SBOM.cdx.json"
fi
