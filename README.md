# ACX Enterprise Contact CLI

> A world-class, production-grade command-line tool to manage contact datasets safely, reproducibly, and at scale — with first-rate OSS governance, security, and documentation.

<!-- Badges (replace repo URL after publishing)
[![CI](https://github.com/ACX-Enterprise-Contact-CLI/acx/actions/workflows/ci.yml/badge.svg)](https://github.com/ACX-Enterprise-Contact-CLI/acx/actions/workflows/ci.yml)
[![License: Apache-2.0](https://img.shields.io/badge/License-Apache--2.0-blue.svg)](./LICENSE)
-->

- **Fast & robust:** C++ (C++17+), parallel processing (`--threads`), atomic writes, graceful signals.
- **Deterministic:** `--deterministic` + `--seed` (or `ACX_RANDOM_SEED`) for reproducible sampling and operations.
- **Auditable:** Machine-readable JSON outputs (`--json`), SHA-256 hashing, local provenance script.
- **Secure & hardened:** FORTIFY, RELRO/NOW, PIE, sanitizers (ASan/UBSan/TSan), distroless container.
- **DX first:** Clear CLI contracts, help in JSON (`help --json`), comprehensive tests, docs, and CI.
- **Governance:** Top-tier Code of Conduct, CONTRIBUTING, SECURITY policy, licensing, CI guidelines.

---

## Table of Contents

- [Features](#features)
- [Quick Start](#quick-start)
- [Install](#install)
  - [From source](#from-source)
  - [Docker](#docker)
  - [Packages (DEB/RPM templates)](#packages-debrpm-templates)
- [Usage](#usage)
  - [Core commands](#core-commands)
  - [Data operations](#data-operations)
  - [Advanced operations](#advanced-operations)
  - [Integrity & reports](#integrity--reports)
  - [Configuration](#configuration)
  - [Performance & determinism](#performance--determinism)
- [JSON Output Contracts](#json-output-contracts)
- [Logging & Exit Codes](#logging--exit-codes)
- [Reproducible Builds & Provenance](#reproducible-builds--provenance)
- [Security & Responsible Disclosure](#security--responsible-disclosure)
- [Contributing & Governance](#contributing--governance)
- [CI “Green checks” on GitHub](#ci-green-checks-on-github)
- [License](#license)

---

## Features

- **CLI you can script:** each command supports human output *and* `--json` (one JSON object/line).
- **Large files:** streaming & reservoir sampling; optional parallelism with `--threads`.
- **Safe writes:** atomic file writes (temp → rename) to prevent corruption.
- **Data hygiene:** validate, dedupe, normalize, redact PII, diff, grep, sort, canonicalize.
- **Integrity:** `hash`, `checksum`, `report` (counts & unique emails).
- **Ops ready:** `doctor`, `selfcheck`, `config init`, Docker `HEALTHCHECK`, distroless image.
- **Build excellence:** CMake presets for Sanitizers/LTO/Coverage; reproducible builds.

---

## Quick Start

```bash
# Build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Try the CLI
./build/acx help
./build/acx help --json
./build/acx report examples/contacts.jsonl --json
Install
From source
Linux/macOS

bash
Copiar código
# deps
# Ubuntu: 
sudo apt-get update && sudo apt-get install -y cmake g++ ninja-build clang-format

# macOS:
# brew install cmake ninja llvm clang-format

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
sudo cp build/acx /usr/local/bin/
Windows (MSVC)

Install Visual Studio with C++ workload and CMake.

Build in “x64 Native Tools Command Prompt”:

bat
Copiar código
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
Docker
bash
Copiar código
# Standard image
docker build -t acx:latest .
docker run --rm -v "$PWD:/work" -w /work acx:latest --version

# Distroless image
docker build -f Dockerfile.distroless -t acx:distroless .
docker run --rm -v "$PWD:/work" -w /work acx:distroless --version
Packages (DEB/RPM templates)
Templates live under packaging/ to build native packages:

Debian/Ubuntu: packaging/debian/

Fedora/EL: packaging/rpm/acx.spec

Use CPack to generate artifacts:

bash
Copiar código
cpack -C Release -B dist
Usage
All commands accept --json for machine-readable output.
Defaults: ACX_DATA_FILE=acx-data.jsonl, ACX_BACKUP_DIR=backups/.

Core commands
bash
Copiar código
acx help
acx help --json            # machine-readable help
acx --version              # also supports --json
acx doctor [--json]        # quick environment sanity checks
acx selfcheck [--json]     # simple CLI health report
acx config init            # write ~/.config/acx/config.json
Data operations
bash
Copiar código
# Validate dataset (name required; if email exists, it must look valid)
acx validate [<file>] [--json]

# Remove duplicates (by email; fallback: entire line)
acx dedupe [<file>] [<out>]

# Timestamped backup to backups/ or custom dir
acx backup [<file>] [<dest-dir>]

# Diff two datasets by email (counts added/removed)
acx diff <A> <B> [--json]

# Redact PII for safe sharing (mask email user, digits in phone)
acx redact [<in>] [<out>]
Advanced operations
bash
Copiar código
# Normalize fields (trim name, lowercase email, digits-only phone w/ leading +)
acx normalize [<in>] [<out>]

# Merge multiple files (dedup by email)
acx merge <out> <file1> <file2> [...]

# Reservoir sample N records (deterministic with seed)
acx sample [<in>] [<out>] [N]

# Regex filter on name/email (case-insensitive)
acx grep [<in>] [<out>] <regex>

# Canonicalize JSON key order (stable diffs/reviews)
acx canonicalize [<in>] [<out>]

# Sort records by key (stable tiebreaker)
acx sort [<in>] [<out>] <key>
Integrity & reports
bash
Copiar código
# SHA-256 of a file (prints "hex  path" or JSON)
acx hash [<file>] [--json]

# Verify integrity against hex or .sha256 file
acx checksum <file> <expected|file.sha256>

# Dataset stats (records, unique emails)
acx report [<file>] [--json]
Configuration
acx config init creates ~/.config/acx/config.json:

json
Copiar código
{
  "data_file": "acx-data.jsonl",
  "backup_dir": "backups",
  "audit_log": "acx-audit.jsonl",
  "lock_dir": ".acx-locks",
  "region": "MX",
  "output": "human"
}
Environment variables
ACX_DATA_FILE, ACX_BACKUP_DIR

ACX_THREADS (see performance)

ACX_RANDOM_SEED (used if --seed not provided)

ACX_LOG_LEVEL = debug|info|warn|error

SOURCE_DATE_EPOCH (build-time define for reproducibility)

Performance & determinism
bash
Copiar código
# Parallelize validate/report with N threads
acx --threads 4 validate examples/contacts.jsonl --json
ACX_THREADS=4 acx report examples/contacts.jsonl --json

# Deterministic sampling
acx --deterministic --seed 42 sample examples/contacts.jsonl out.jsonl 100
JSON Output Contracts
General rule: --json emits one JSON object/line (easy to pipe/parse). Examples:

bash
Copiar código
acx report examples/contacts.jsonl --json
# {"file":"examples/contacts.jsonl","records":1234,"unique_emails":1201}

acx doctor --json
# {"doctor":"ok","home":true,"tmp":true,"acx_config":true}

acx hash examples/contacts.jsonl --json
# {"file":"examples/contacts.jsonl","sha256":"<64-hex>"}
Logging & Exit Codes
Logging

Set ACX_LOG_LEVEL to debug|info|warn|error.

Windows console supports VT/ANSI when available; disable color with NO_COLOR or CLI flag if provided.

Exit codes (subset)

0 — success

2 — usage error (bad flags/args)

3 — validation error

4 — I/O error

1 — general failure

Reproducible Builds & Provenance
bash
Copiar código
# Reproducible builds (stable timestamps)
export SOURCE_DATE_EPOCH=$(git log -1 --pretty=%ct)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Generate provenance for dist artifacts
cpack -C Release -B dist
python3 scripts/provenance_local.py dist > provenance.json
See docs/REPRODUCIBLE_BUILDS.md and docs/INTEGRITY_AND_REPORTS.md.

Security & Responsible Disclosure
Please report vulnerabilities privately to: security@acx.dev.
Do not open public issues for sensitive reports.
We aim to acknowledge within 72 hours and provide an initial assessment within 7 days.

See SECURITY.md for full policy.

Contributing & Governance
Read CONTRIBUTING.md (style, tests, DCO, PR process).

Participation is governed by CODE_OF_CONDUCT.md.

License headers: SPDX-License-Identifier: Apache-2.0.

CI “Green checks” on GitHub
To avoid red ❌ while keeping strong signal:

In Branch protection → Required status checks, mark only ci (build + tests).

Lints/sanitizers/scans remain non-blocking and skip on fork PRs (no secrets).

See docs/CI_HEALTH.md for a step-by-step setup.

License
This project is licensed under the Apache License 2.0.
See LICENSE for details.

Credits
Community impact guidelines adapted from Mozilla.

Code of Conduct adapted from Contributor Covenant v2.1 (CC BY 4.0).

Thank you to all contributors who help keep ACX world-class.

Project home: https://github.com/ACX-Enterprise-Contact-CLI/acx (replace with your repository URL)