# Changelog

All notable changes are documented here following Keep a Changelog.
This project uses Semantic Versioning.

## [1.0.0] - 2025-08-19
### Added
- Initial public release of AgendaCyberneticX Enterprise CLI with storage, validation, history, backups, CSV/JSON import-export.

## [1.1.1] - 2025-08-19
### Added
- Structured logging via env (`ACX_LOG_LEVEL`, `ACX_LOG_FORMAT`)
- Fast lookup indexes for email/phone (O(1) avg)

### Improved
- CI matrix for Linux/macOS/Windows and artifact upload


## [2.0.0] - 2025-08-19
### Added
- Persistent on-disk index (`acx-index.json`) keyed by data hash
- HMAC-SHA256 signing for backups and `verify-backup` command
- SBOM generator (SPDX/CycloneDX) without external deps
- `metrics` command for quick health snapshot

### Improved
- Startup time on large datasets via persistent index reuse
- Security posture via signed backups


## [3.0.0] - 2025-08-19
### Added
- Parallel import (`--parallel`) with deterministic order and global dedup (thread-safe)
- POSIX `mmap` read with Windows fallback
- OTEL exporter (OTLP-like JSON) via `ACX_OTEL=1`
- Perf smoke script and opt-in gate in CI (`PERF_GATE_MS`)
- Env controls: `ACX_IMPORT_WORKERS`, `ACX_IMPORT_MAX`, `ACX_RECORD_MAX_SIZE`

### Improved
- Documentation on performance and observability


## [4.0.0] - 2025-08-19
### Added
- RFC4180-compliant CSV parser
- Binary on-disk index (`acx-index2.bin`) enabling fast search without full load (flag: `ACX_FAST_SEARCH=1`)
- Release workflow with checksums + SBOM (SPDX)
- Frontier Certificate document

### Improved
- Save path now builds both hash-coupled JSON index and binary search index
- README updated with Fast Search and CSV details


## [5.0.0] - 2025-08-20
### Added
- **Pluggable storage engines**: JSONL (default) and optional SQLite (if detected)
- Optional **Zstandard** compression for backups (`ACX_ZSTD=1`)
- Man page (`docs/man/acx.1`), install rules and **CPack** artifacts (TGZ/ZIP)
- `.clang-format` and `.clang-tidy` for consistent style and static checks

### Improved
- CLI can select engine via `--engine`/`ACX_ENGINE`
- Supply-chain posture via SLSA release workflow (see `.github/workflows/release.yml`)


## [6.0.0] - 2025-08-20
### Added
- **JSON output** mode for `list`/`search` (`--output json` or `--json`).
- **Fuzzing** (libFuzzer) target for CSV; GitHub Actions `fuzz.yml` (60s smoke).
- **Coverage** workflow with llvm-cov report.
- **CodeQL** and **Gitleaks** workflows.
- Full OSS governance & policy docs (CoC, CONTRIBUTING, SECURITY, GOVERNANCE, etc.).

### Improved
- Documentation suite (WHITEPAPER, MARKETING, ACCESSIBILITY, SUSTAINABILITY, TRANSPARENCY, AUDIT).


## [7.0.0] - 2025-08-20
### Added
- RFC4180 CSV import path (JSONL & SQLite engines)
- `merge` command with strategies (skip|prefer-newer|prefer-existing)
- `doctor` command for health checks
- Cross-process repo lock to serialize write ops
- Shell completions (bash/zsh)
- Cosign keyless signing workflow for release assets
- Reproducible build flags in CMake


## [8.0.0] - 2025-08-20
### Added
- Optional **RocksDB** storage engine with secondary indexes via keyspace (`email:*`, `phone:*`).
- Commands: **validate**, **diff**, **verify-file**, **prune**; global **--dry-run**.
- **policy.json** support with domain allowlist and minimum name length.
- JSONL engine **journal** (append-only ops log).

### Improved
- Help text and docs updated to reflect new functionality.


## [9.0.0] - 2025-08-20
### Added
- Commands: **selftest**, **determinism-check**.
- Build hardening flags via `ACX_HARDENED` (RELRO/NOW, SSP, FORTIFY, PIE).
- Workflows: **frontier-check**, **Scorecard**, **verify-signatures**.
- Docs: THREAT_MODEL.md, ASSURANCE_CASE.md, FRONTIER_CERTIFICATE.md, DETERMINISM.md.
- Example **policy.json**.

### Assurance
- Frontier certificate issued for the defined scope.

## [2025-08-27] - Continuous hardening
- Added shell completions, CPack packaging, SBOM/Provenance/Scorecards/Trivy/e2e workflows.

### OMEGA hardening
- Docs site (MkDocs) & API docs (Doxygen) pipelines
- Lint & CI matrix with sanitizers and ccache
- Release Drafter, Dependabot, Repro builds helper
- Packaging templates (Homebrew, winget)

### SINGULARITY hardening — 2025-08-27
- GitHub Pages deploy + Docs site
- Benchmarks job with JSON artifact
- MSan, Cppcheck, Commitlint, Workflow policy (OPA)
- Issue/PR templates, Incident response & Deprecation policy
- ADRs/RFCs, pre-commit hooks extended, CPack DEB/RPM
- Fuzz corpora, labeler, packaging templates

### TRANSCENDENCE hardening — 2025-08-27
- CMake hardening, LTO, WERROR, CMake package & pkg-config
- Devcontainer, MUSL Dockerfile (static), OSV & Semgrep
- Valgrind, DCO gate, Coverage-gate 85% + Codecov, IWYU
- Compliance map, performance budget docs

### ABSOLUTE hardening — 2025-08-27
- CMakePresets, Nix flake, Makefile
- Bench baseline & gate, Hadolint, Codespell, Lychee, Yamllint, Trivy FS
- Cosign-ready signing workflow (gated), CLI accessibility guide, SDLC, review checklist
- Seccomp profile & security hardening guide

### INFINITY hardening — 2025-08-27
- REUSE lint, Gitleaks, Dependency review
- help2man manpage generation, ClamAV scan
- Markdownlint/Prettier, Release Please automation
- Mermaid diagrams, data retention & security review docs
- CODEOWNERS, contributors list, OpenSSF mapping, flamegraph tooling

### PERFECTION hardening — 2025-08-27
- vcpkg & Conan manifests; toolchain files (clang/mingw)
- PGO workflow; SBOM diff; licensee verification
- Editor & docker hygiene (.editorconfig, .dockerignore)
- Community docs (MAINTAINERS, TRIAGE, ROADMAP, BACKPORTS)
- Binary size budget doc

### ULTIMATE hardening — 2025-08-27
- Fish completion + install rules
- ShellCheck, scan-build, coverage HTML, CLI smoke tests
- CMake formatting check, ops/compliance docs, funding placeholder
- Release verification script

### MYTHIC hardening — 2025-08-27
- Dist checksums workflow; Windows MSVC /analyze (non-blocking)
- Contract tests (semver) and pre-commit hooks
- Risk register & Disaster Recovery plan; OCI labels in Dockerfile
- CITATION.cff (if missing)

### APEX hardening — 2025-08-27
- Support & Privacy docs; I18N guidance
- Preflight toolchain check; secure docker-compose
- Snapcraft & Arch packaging templates
- Pre-commit CI; .gitattributes; blame-ignore
- Release policy & Changelog guide; explicit CodeQL config
- Rootless Docker guide; Symbols guide; reproducibility checker; Style guide

### ZENITH hardening — 2025-08-27
- Generated version header & compile_commands.json
- VSCode/clangd configs; actions pinning check
- Windows NSIS template; AppImage notes
- Integration examples (CMake/pkg-config); exit codes spec; SBOM JSON validation

### PINNACLE hardening — 2025-08-27
- Static/Tiny build toggles; build presets doc
- Multi-arch Docker buildx (artifact), cross-OS release binaries workflow
- Inclusion & Accessibility statements; Ethical guidelines; Security advisory template
- Third-party notices generator from SBOM; SLOs & metrics guidance; SECURITY.txt
- Makefile 'dist' target chaining checksums and verification

### EVEREST hardening — 2025-08-27
- Build-info header (compiler/system/git)
- Distroless Dockerfile; SPDX header enforcer
- Offline install script; release checklist; migration guide; security FAQ; artifact retention policy
- MkDocs nav updated with integration/ops/security sections

### SUMMIT hardening — 2025-08-27
- .clang-format; AppArmor & SELinux templates + docs
- JSON Schema for contacts + DATA_SCHEMA.md; example CSV/JSONL datasets
- SBOM license allowlist gate; systemd service example
- Compatibility matrix; enhanced PR template; .gitignore; Makefile all-checks

### AURORA hardening — 2025-08-27
- JSON configuration support (ACX_CONFIG / XDG / ~/.config)
- Storage reads env overrides in constructor
- CLI auto-loads and applies config at startup
- Exit codes header for consistency
- New tests: config env override & config loader
- Example config and documentation (CONFIGURATION.md)

### ASCENT hardening — 2025-08-27
- MSVC hardening flags (/guard:cf, /Qspectre, /sdl, CETCOMPAT)
- Implemented `--version --json` output using build info and version headers
- New test `test_version_json` validating JSON shape
- Docs: JSON_OUTPUTS.md

### APOTHEOSIS hardening — 2025-08-27
- GCC/Clang hardening & pedantic flags, optional Werror
- Graceful SIGINT/SIGTERM handling; Windows UTF-8 + VT console init
- Docker HEALTHCHECK in images
- Docs: HARDENING_FLAGS.md and INTERRUPTS_AND_WINDOWS.md

### OMEGA hardening — 2025-08-27
- New CLI subcommands: doctor, selfcheck, config init
- Debian & RPM packaging templates
- clang-format CI check; fuzz seed corpora
- Tests for doctor/selfcheck/config init; docs for new commands

### HYPERION hardening — 2025-08-27
- New CLI: `hash` (SHA-256) & `report` (basic stats) with `--json`
- POSIX locale init; docs for integrity & reports
- Tests for `hash` and `report`; local provenance script

### TITAN hardening — 2025-08-27
- CLI data-ops: validate, dedupe, backup, diff, redact
- Tests for validate/dedupe/backup; CI smoke for data-ops
- Docs: DATA_OPERATIONS.md

### COLOSSUS hardening — 2025-08-27
- New CLI: normalize, merge, sample, grep, checksum (with atomic writes)
- Sanitizers/LTO/Coverage build toggles + sanitizer CI
- Tests for normalize/merge/checksum; docs for advanced data operations and sanitizers

### MONOLITH hardening — 2025-08-27
- Global `--threads` and `ACX_THREADS`; parallel `validate`/`report`
- `help --json` for machine-readable CLI spec
- Reproducible builds via `SOURCE_DATE_EPOCH` and docs
- Codespell CI; performance bench script; tests for help-json and threads
