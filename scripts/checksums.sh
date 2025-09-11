#!/usr/bin/env bash
set -euo pipefail
mkdir -p dist
if command -v sha256sum >/dev/null 2>&1; then
  sha256sum dist/* > dist/SHA256SUMS.txt || true
fi
if command -v shasum >/dev/null 2>&1; then
  shasum -a 512 dist/* > dist/SHA512SUMS.txt || true
fi
echo "Checksums written to dist/SHA256SUMS.txt and dist/SHA512SUMS.txt (when tools available)."
