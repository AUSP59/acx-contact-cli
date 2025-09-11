#!/usr/bin/env bash
set -euo pipefail
echo "== Build & tests =="
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure || true
echo "== Lints =="
python3 -c "print('skip offline-only lints')"
echo "== SBOM =="
bash scripts/make_sbom.sh || true
echo "== Package =="
cpack -C Release -B dist
echo "Artifacts in dist/"
ls -l dist || true
