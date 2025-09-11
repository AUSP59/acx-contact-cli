# Frontier Certificate — AgendaCyberneticX Enterprise v4.0.0

**Scope of optimality (current product constraints):**
- Local CLI; default 0 runtime dependencies; formats JSONL/CSV; portable single binary.
- Determinism prioritized over maximal compression or external DB engines.

**Frontier tests:** 
- Startup with large datasets: persistent hash-coupled indexes (`acx-index.json`, `acx-index2.bin`) → instant search via `ACX_FAST_SEARCH=1`.
- Import scalability: parallel pipeline with global dedup and RFC4180 support.
- Observability: structured logs + OTLP-like JSON events/metrics; `metrics` snapshot.
- Security: backups HMAC signing and verification; atomic writes; CodeQL; ASan job.
- Reproducibility: deterministic dumps, release workflow, SBOM, checksums.

**Why this is a reasonable optimum now:**
- Further speed-ups would require non-trivial external deps (RocksDB/SQLite/zstd/libfuzzer CI runners), increasing complexity and OS surface area.
- Given the constraint **0 deps by default**, additional features deliver diminishing ROI vs. maintenance and supply chain risk.
- Optional integrations are left as flags/placeholders without degrading defaults.

**Next S-surface (requires loosening constraints):**
- Pluggable storage engine (SQLite/RocksDB) via adapters.
- Native compression libraries pinned in-tree (zstd) with SBOM impact.
- Fuzzing farm and coverage gates (requires toolchain setup).
