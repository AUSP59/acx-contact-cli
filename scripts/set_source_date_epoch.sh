#!/usr/bin/env bash
set -euo pipefail
export SOURCE_DATE_EPOCH=$(git log -1 --pretty=%ct)
echo "SOURCE_DATE_EPOCH=$SOURCE_DATE_EPOCH"
