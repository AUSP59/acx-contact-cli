# Assurance Case (GSN-style, prose)

Goal: ACX CLI is functionally complete, safe-by-default, and reproducible within its defined scope.

Strategy:
- Demonstrate completeness via command matrix and self-tests.
- Demonstrate safety via hardening flags, lock/journal, validation.
- Demonstrate reproducibility via CI workflows and determinism checks.

Evidence:
- `acx selftest` and `acx determinism-check` return success.
- CI `frontier-check` passes on PRs.
- Signed artifacts (`sign.yml`) and verified (`verify-signatures.yml`).

Assumptions:
- Compiler/toolchain honors flags; OS provides expected syscalls.
- RocksDB/SQLite present only when selected.
