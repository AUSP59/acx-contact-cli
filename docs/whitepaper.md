# Whitepaper — ACX Enterprise Contact CLI

**Abstract.** ACX is a high-assurance CLI for contact management. It combines a minimal-dependency, portable C++ core with defense-in-depth (CodeQL, fuzzers, sanitizers), transparent release artifacts (SBOM, provenance), and governance suitable for enterprise adoption.

## Guarantees
- Deterministic, cross-platform builds via CMake; MUSL-friendly container.
- Zero runtime telemetry; file-based storage by default.
- Optional backends (SQLite/RocksDB) gated by explicit flags.

## Security
- Private vulnerability reporting; STRIDE-informed threat model.
- SBOM and SLSA provenance for releases; container scans (Trivy).

## Limitations
- RocksDB backend requires external dependency.
- libFuzzer fuzzing requires clang.

## Future Work
- Keyless signing (Sigstore cosign) for release artifacts.
- Reproducible-build attestations and supply-chain policies.
