#!/usr/bin/env bash
set -euo pipefail
echo "Checking toolchain..."
command -v cmake >/dev/null || { echo "cmake missing"; exit 1; }
command -v c++ >/dev/null || { echo "C++ compiler missing"; exit 1; }
echo "OK"
