# ACX Contact CLI

![License: Apache-2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg) ![C++20](https://img.shields.io/badge/C%2B%2B-20-blue) ![CMake ≥ 3.16](https://img.shields.io/badge/CMake-%E2%89%A53.16-brightgreen) ![OS](https://img.shields.io/badge/OS-Linux%20%7C%20macOS%20%7C%20Windows-informational)

Fast, secure C++20 CLI to process large contact datasets. Validate and normalize fields, deduplicate records, redact PII, and import/export CSV or JSONL with optional pluggable storage (JSONL, SQLite, RocksDB), compression (Zstandard), crypto (OpenSSL), phone parsing (libphonenumber), metrics (OpenTelemetry), and rich shell completions.

- Source: https://github.com/AUSP59/acx-contact-cli
- Maintainer: Alan (AUSP59) • alanursapu@gmail.com

## Table of contents
- Features
- Quick start
- Installation
  - From source
  - Windows notes
- Optional dependencies
- Configuration
- Usage
  - Commands
  - Examples
- Storage backends
- Performance tips
- Security and privacy
- Metrics and tracing
- Shell completions
- Testing and fuzzing
- Packaging
- Contributing
- License
- Acknowledgments

## Features
- High-volume CSV/JSONL ingestion with stream processing and constant-memory writers.
- Field validation: email, phone, names, country/ISO codes, timestamps.
- Normalization: case folding, Unicode NFC, whitespace trimming, locale-aware rules.
- Deduplication: exact and fuzzy (configurable keys, similarity threshold).
- PII redaction/masking with reversible hashing or irreversible wipe.
- Import/Export: CSV ↔ JSONL; optional SQLite/RocksDB stores for indexing.
- Compression: transparent Zstd read/write when enabled.
- Reliability: atomic writes, crash-safe temp files, checksums.
- Observability: structured logs, metrics, and optional OpenTelemetry traces.
- Cross-platform: Linux, macOS, Windows. Single static binary possible.
- Quality: unit/contract tests, sanitizers, fuzz harnesses.
- Developer ergonomics: rich help, JSON help, shell completions, self-check.

## Quick start
    acx --version
    acx help
    acx help --json

Process a CSV and write a normalized, deduplicated JSONL:
    acx normalize --in contacts.csv --out normalized.jsonl --format jsonl
    acx dedupe --in normalized.jsonl --out unique.jsonl --key email --key phone --similarity 0.92

Validate and redact PII before export:
    acx validate --in unique.jsonl --fail-on error
    acx redact --in unique.jsonl --out public.csv --format csv --mask email:partial --mask phone:last4

## Installation

### From source
Prerequisites:
- CMake ≥ 3.16
- A C++20 compiler (GCC 11+, Clang 13+, MSVC 19.3+)
- Git, Python 3 (for helper scripts)
- Optional deps via your package manager or vcpkg

Build (Unix-like):
    git clone https://github.com/AUSP59/acx-contact-cli.git
    cd acx-contact-cli
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j
    ./build/acx --version

Build (Windows, PowerShell; Visual Studio 2022):
    git clone https://github.com/AUSP59/acx-contact-cli.git
    cd acx-contact-cli
    cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release
    .\build\Release\acx.exe --version

### Windows notes
- Use the x64 Native Tools prompt or install Ninja and set -G "Ninja".
- Long paths may require enabling Windows long path support.

## Optional dependencies
Enable features at configure time. The CLI runs without them using JSONL storage.

Common CMake switches:
    -DACX_WITH_SQLITE=ON        # SQLite3 storage
    -DACX_WITH_ROCKSDB=ON       # RocksDB storage
    -DACX_WITH_ZSTD=ON          # Zstandard compression
    -DACX_WITH_OPENSSL=ON       # Crypto utilities
    -DACX_WITH_PHONENUMBER=ON   # libphonenumber validation
    -DACX_WITH_OTEL=ON          # OpenTelemetry
    -DBUILD_TESTING=ON          # Unit/integration tests
    -DBUILD_FUZZERS=ON          # Fuzz harnesses
    -DACX_ENABLE_ASAN=ON        # AddressSanitizer (debug)
    -DACX_ENABLE_UBSAN=ON       # UndefinedBehaviorSanitizer
    -DACX_ENABLE_LTO=ON         # Link-time optimization

Using vcpkg toolchain:
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DACX_WITH_SQLITE=ON -DACX_WITH_ZSTD=ON

## Configuration
CLI flags override env and config file. A minimal config.yaml:

    io:
      input_format: csv
      output_format: jsonl
      delimiter: ","
      has_header: true
    normalize:
      email_lowercase: true
      name_nfc: true
      country_to_iso2: true
    dedupe:
      keys: ["email", "phone"]
      similarity: 0.92
    redact:
      email: "partial"   # none|partial|hash|wipe
      phone: "last4"
    storage:
      driver: "jsonl"    # jsonl|sqlite|rocksdb
      uri: "./data"
    telemetry:
      enabled: false

Point the CLI to a config:
    acx --config config.yaml normalize --in contacts.csv --out normalized.jsonl

Environment variables:
    ACX_LOG_LEVEL=info
    ACX_MAX_WORKERS=8
    ACX_TMPDIR=/fast/tmp
    OTEL_EXPORTER_OTLP_ENDPOINT=http://localhost:4317

## Usage

### Command synopsis
    acx [global flags] <command> [command flags]

Global flags:
    --help, -h
    --version
    --config <file>
    --log-level <trace|debug|info|warn|error>
    --metrics <json|prom|otel>

### Commands
- validate        Validate records and report errors/warnings.
- normalize       Normalize fields (email, names, phones, country codes).
- dedupe          Remove duplicates using keys and fuzzy matching.
- redact          Mask or remove PII.
- import          Load data into a storage backend.
- export          Dump data from storage to CSV/JSONL.
- stats           Compute column stats and quality metrics.
- selfcheck       Run a quick environment and I/O sanity check.
- completion      Emit shell completion scripts.

### Examples

Normalize then validate:
    acx normalize --in contacts.csv --out normalized.jsonl
    acx validate --in normalized.jsonl --fail-on error

Deduplicate with composite key:
    acx dedupe --in normalized.jsonl --out unique.jsonl --key email --key phone --similarity 0.9

Redact with hashing:
    acx redact --in unique.jsonl --out safe.jsonl --email hash --phone wipe

Import to SQLite and export to CSV:
    acx import --in unique.jsonl --storage sqlite --uri contacts.db
    acx export --storage sqlite --uri contacts.db --out final.csv --format csv

Generate PowerShell completion:
    acx completion pwsh > acx.ps1

## Storage backends
- JSONL: newline-delimited JSON. Good for streaming and diff-friendly repos.
- SQLite: single-file DB. Fast lookups and dedupe indexes.
- RocksDB: LSM-tree KV store for very large datasets.

URIs:
    jsonl: ./data_dir
    sqlite: ./contacts.db
    rocksdb: ./rocks_dir

## Performance tips
- Prefer Release builds with -DACX_ENABLE_LTO=ON.
- Use fast local SSD for tmp/output. Set ACX_TMPDIR.
- Enable Zstd for large CSVs to save I/O.
- For fuzzy dedupe, start at similarity 0.95 and lower only if recall is insufficient.

## Security and privacy
- No network access by default. Telemetry is opt-in.
- Writes are atomic with temp files and fsync barriers.
- Use irreversible wipe for PII when compliance requires it.
- Secrets never logged. Redaction applies to error paths.
- SBOM and license report scripts are provided under scripts/.

## Metrics and tracing
- --metrics prom exposes Prometheus text on stdout for scraping via redirection.
- With -DACX_WITH_OTEL=ON set OTEL_* env vars to export spans/metrics.

## Shell completions
    # bash
    acx completion bash > /etc/bash_completion.d/acx
    # zsh
    acx completion zsh > "${fpath[1]}/_acx"
    # fish
    acx completion fish > ~/.config/fish/completions/acx.fish
    # PowerShell
    acx completion pwsh | Out-String | Invoke-Expression

## Testing and fuzzing
Build and run tests:
    cmake -S . -B build -DBUILD_TESTING=ON
    cmake --build build -j
    ctest --test-dir build --output-on-failure

Optional fuzz targets:
    cmake -S . -B build -DBUILD_FUZZERS=ON -DACX_ENABLE_ASAN=ON -DACX_ENABLE_UBSAN=ON

## Packaging
- AppImage, Homebrew, and Chocolatey manifests live under packaging/.
- Container image Dockerfile is provided. Healthcheck should use acx --version.
- Release artifacts should include checksums and SBOM.

Build a minimal container:
    docker build -t acx:local .
    docker run --rm acx:local acx --version

## Contributing
- Use clang-format and clang-tidy before PRs.
- Commit messages: Conventional Commits.
- Open issues with minimal repro datasets when possible.

Dev loops:
    python3 scripts/format_fix.py
    python3 scripts/run_tidy.py
    cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build build -j
    ctest --test-dir build

## License
Apache License 2.0. See LICENSE.

## Acknowledgments
This project uses excellent open-source components: SQLite, RocksDB, Zstandard, OpenSSL, libphonenumber, fmt, spdlog, and more. Thank you to all contributors.

---
Copyright © AUSP59