# Contributing to ACX Enterprise Contact CLI

Thank you for your interest in contributing! This document explains how to propose changes, report issues, and develop features to the highest global standard.

> **Code of Conduct**  
> Participation in this project is governed by our [Code of Conduct](./CODE_OF_CONDUCT.md). By contributing, you agree to uphold it.

---

## Table of Contents
1. [Ways to Contribute](#ways-to-contribute)
2. [Project Architecture](#project-architecture)
3. [Development Environment](#development-environment)
4. [Build, Test & Run](#build-test--run)
5. [Style, Linting & Formatting](#style-linting--formatting)
6. [Commit Message & PR Guidelines](#commit-message--pr-guidelines)
7. [Adding/Changing CLI Commands](#addingchanging-cli-commands)
8. [Testing Requirements](#testing-requirements)
9. [Performance & Determinism](#performance--determinism)
10. [Documentation Standards](#documentation-standards)
11. [Issue Triage](#issue-triage)
12. [Security & Responsible Disclosure](#security--responsible-disclosure)
13. [Licensing & DCO](#licensing--dco)
14. [Release Workflow](#release-workflow)
15. [Governance](#governance)

---

## Ways to Contribute
- **Issues**: Bug reports, feature requests, and RFCs (well-scoped and reproducible).
- **Pull Requests**: Code, tests, docs, CI improvements.
- **Operations**: Packaging, CI/CD, reproducible builds, security hardening.
- **Docs**: Tutorials, troubleshooting, accessibility improvements.

Before starting non-trivial work, open an **Issue** or **Discussion** to align.

---

## Project Architecture
- Language: **C++** (C++17 or later).
- Build: **CMake** + Ninja (recommended).
- Binary: `acx` (CLI).
- Key directories:
  - `src/` — Core CLI code.
  - `include/` — Public/internal headers (SPDX headers required).
  - `tests/` — Unit and integration tests (CTest).
  - `docs/` — End-user and developer documentation.
  - `packaging/` — DEB/RPM spec/templates.
  - `.github/workflows/` — CI pipelines.
  - `examples/` — Example datasets and usage.

---

## Development Environment

### Prerequisites
- **Ubuntu**: `sudo apt-get update && sudo apt-get install -y cmake g++ ninja-build clang-format`
- **macOS**: `brew install cmake ninja llvm clang-format`
- **Windows (MSVC)**: Install **Visual Studio** with C++ workload; add `cmake` & `ninja` (e.g., `choco install cmake ninja`).

> Ensure `clang-format` is installed for formatting checks.

### Optional Tools
- Sanitizers: GCC/Clang with `-fsanitize` (Address/UB/Thread).
- `codespell` for documentation quality: `pipx install codespell`.

---

## Build, Test & Run

```bash
# Configure & build (Release)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure

# Run the CLI
./build/acx help
./build/acx help --json
Useful Build Options (CMake)
-DACX_PEDANTIC=ON (default) — -Wall -Wextra -Wpedantic -Wshadow ...

-DACX_WERROR=ON — Treat warnings as errors.

-DACX_ASAN=ON -DACX_UBSAN=ON — Sanitizers.

-DACX_TSAN=ON — ThreadSanitizer (race detection).

-DACX_LTO=ON — Link Time Optimization.

-DACX_COVERAGE=ON — Coverage flags (Debug builds recommended).

Reproducible Builds
bash
Copiar código
export SOURCE_DATE_EPOCH=$(git log -1 --pretty=%ct)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
Style, Linting & Formatting
Formatting: clang-format (the CI will fail on style drift).

SPDX headers (Apache-2.0) required at the top of all source and header files.

Naming & Structure: Prefer small, testable units; avoid global state.

Error handling: Return well-defined exit codes; never crash on malformed input.

I/O contracts: All new commands must support --json machine-readable output.

Run locally:

bash
Copiar código
clang-format -i $(git ls-files '*.cpp' '*.hpp')
codespell
Commit Message & PR Guidelines
Conventional Commits
Use Conventional Commits:

sql
Copiar código
feat(cli): add `canonicalize` command with stable key order
fix(normalize): preserve leading plus on E.164 phone numbers
docs: expand determinism guide
test(report): add unique email counting test
ci: add concurrency and fork PR skip
DCO Sign-Off (required)
Every commit must include:

pgsql
Copiar código
Signed-off-by: Your Name <you@example.com>
Add automatically with git commit -s.

PR Checklist
 Code formatted (clang-format).

 Tests added/updated and passing (ctest).

 New/changed CLI commands documented in docs/.

 CHANGELOG.md updated (keep a clear, human-readable entry).

 --json added where applicable; no breaking changes without RFC.

 Performance not regressed (scripts/bench_cli.py if relevant).

Large changes: open an RFC issue first.

Adding/Changing CLI Commands
Requirements
Human & JSON outputs (--json returns a single JSON object or array on one line).

Follow the existing parsing pattern in src/cli.cpp.

Use atomic writes for files (write to temp → rename()).

Respect environment variables:

ACX_DATA_FILE, ACX_BACKUP_DIR

ACX_THREADS / --threads N

ACX_RANDOM_SEED / --deterministic --seed N

SOURCE_DATE_EPOCH (compile-time define)

Steps
Add helpers (pure functions) with clear contracts.

Wire the command in the main dispatch loop.

Add tests in tests/ (include both success and failure cases).

Document usage in docs/ (examples for both human/JSON outputs).

Update CHANGELOG.md.

Testing Requirements
Unit/Functional tests for every new feature or bugfix.

Negative tests for invalid inputs.

Determinism tests for randomized features (use --deterministic --seed N).

Run sanitizers locally when possible:

bash
Copiar código
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=RelWithDebInfo -DACX_ASAN=ON -DACX_UBSAN=ON
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
Performance & Determinism
Prefer streaming, O(1) extra memory when feasible on large files.

Use --threads N (or ACX_THREADS) for parallelizable workloads (validate, report).

Keep randomized flows deterministic under --deterministic --seed N.

Benchmark if your change touches hot paths:

bash
Copiar código
python3 scripts/bench_cli.py
python3 scripts/bench_cli.py 4
Documentation Standards
Update relevant guides in docs/.

Keep README.md concise; link to deep docs.

Include examples users can copy-paste.

Accessibility: write clear headings, short paragraphs, and code blocks for commands.

Issue Triage
Type: bug, feature, docs, ci, security.

Priority:

P0: security regression / data loss.

P1: production-impacting bug.

P2: quality-of-life improvement.

P3: cosmetic or low impact.

Add labels, repro steps, and acceptance criteria.

Security & Responsible Disclosure
Report vulnerabilities privately to security@acx.dev.

Do not open public issues for sensitive reports.

We aim to acknowledge within 72 hours and provide an initial assessment within 7 days.

See SECURITY.md for detailed policy.

Licensing & DCO
License: Apache-2.0. Include SPDX-License-Identifier: Apache-2.0 in new files.

All contributions must be DCO-signed (git commit -s).

Release Workflow
SemVer: MAJOR.MINOR.PATCH.

Update CHANGELOG.md and version metadata.

Tag releases: git tag -a vX.Y.Z -m "Release vX.Y.Z".

Build artifacts with CPack and container images.

(Optional) Package templates: packaging/debian/, packaging/rpm/.

Governance
Maintainers: Review, triage, and decide on scope and roadmap.

Contributors: Open PRs, propose RFCs, and improve docs/tests.

Decisions prioritize:

Security & stability

User experience & data integrity

Maintainability & transparency

CI Notes (No Red X on GitHub)
The only Required check should be ci (build + tests).

Lints/sanitizers/coverage/security scans are informational and non-blocking by default.

Fork PRs: heavy jobs skip safely (no secrets).

See docs/CI_HEALTH.md for a 2-minute setup.

Thank you for helping us keep this project world-class!
If you have questions: open a Discussion, or email conduct@acx.dev / security@acx.dev.