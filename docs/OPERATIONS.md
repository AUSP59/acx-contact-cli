# Operations Runbook
- Build & Test: `cmake -S . -B build && cmake --build build && ctest --test-dir build`
- Release: tag `vX.Y.Z` -> CI builds, packages, provenance, SBOM
- Triage: follow `TRIAGE.md`; security incidents `INCIDENT_RESPONSE.md`
- Size & perf: `SIZE_BUDGET.md`, `PERF_BUDGET.md`, `bench_gate.py`
