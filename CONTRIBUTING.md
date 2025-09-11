# Contributing Guide — ACX Contact CLI

Maintainer: Alan (AUSP59) • alanursapu@gmail.com  
License: Apache-2.0

Thank you for improving ACX. This guide explains how to set up the dev environment, propose changes, and get your PR merged.

## Table of Contents
- Ground Rules
- What to Work On
- Development Setup
- Build and Test
- Linters and Formatters
- Fuzzing and Sanitizers
- Commit Messages
- Branching and PRs
- Review Checklist
- Documentation
- Performance and Benchmarks
- Security and Responsible Disclosure
- Release and Changelog
- Code of Conduct
- Attribution

## Ground Rules
- Be respectful and concise.  
- Prefer small, focused PRs.  
- Add or update tests with behavior changes.  
- Keep the CLI stable and predictable.  
- Do not include private data in code, tests, or logs.

## What to Work On
- Good first issues and help wanted are labeled in the tracker.  
- If unsure, open a discussion or a draft issue before coding.

## Development Setup
Prerequisites:
- CMake ≥ 3.16
- C++20 compiler (GCC 11+, Clang 13+, MSVC 19.3+)
- Git, Python 3
- Optional: vcpkg for dependencies (SQLite3, RocksDB, Zstandard, OpenSSL, libphonenumber), pre-commit

Clone:
    git clone https://github.com/AUSP59/acx-contact-cli.git
    cd acx-contact-cli

Configure (Unix-like):
    cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo

Configure (Windows, VS 2022):
    cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=RelWithDebInfo

Using vcpkg (optional):
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake -DACX_WITH_SQLITE=ON -DACX_WITH_ZSTD=ON

## Build and Test
Build:
    cmake --build build -j

List tests:
    ctest --test-dir build -N

Run tests:
    ctest --test-dir build --output-on-failure

Enable tests at configure time if needed:
    cmake -S . -B build -DBUILD_TESTING=ON

## Linters and Formatters
Format all sources:
    git ls-files '*.h' '*.hpp' '*.c' '*.cc' '*.cpp' | xargs clang-format -i

Static checks (examples):
    cppcheck --enable=warning,performance,portability --std=c++20 --project=build/compile_commands.json
    run-clang-tidy -p build

Optional: pre-commit. Create .pre-commit-config.yaml, then:
    pip install pre-commit
    pre-commit install
    pre-commit run --all-files

## Fuzzing and Sanitizers
Configure with sanitizers:
    cmake -S . -B build-asan -DACX_ENABLE_ASAN=ON -DACX_ENABLE_UBSAN=ON -DBUILD_TESTING=ON
    cmake --build build-asan -j
    ctest --test-dir build-asan --output-on-failure

Fuzz targets (if available):
    cmake -S . -B build-fuzz -DBUILD_FUZZERS=ON -DACX_ENABLE_ASAN=ON -DACX_ENABLE_UBSAN=ON
    cmake --build build-fuzz -j
    ./build-fuzz/fuzz_csv
    ./build-fuzz/fuzz_jsonl

## Commit Messages
Use Conventional Commits:
- feat: new user-visible feature
- fix: bug fix
- perf: performance improvement
- refactor: code change without behavior change
- docs: documentation only
- test: tests only
- build/chore/ci: infra changes

Example:
    feat(normalize): add Unicode NFC pass for names

DCO sign-off required. Append:
    Signed-off-by: Your Name <your.email@example.com>

## Branching and PRs
Branching:
- Create feature branches off main: feature/<short-description>
- Keep branches rebased when possible.

PR expectations:
- Green CI on Linux, macOS, Windows.
- Tests added/updated.
- Docs updated (README, usage help, man/completions if applicable).
- No unrelated formatting diffs.
- One logical change per PR.

Open a draft PR early if you want feedback.

## Review Checklist
Author self-check before requesting review:
- Builds cleanly with -DCMAKE_BUILD_TYPE=Release.
- No new warnings on default compilers.
- clang-format applied.
- New code covered by tests; all tests pass locally.
- Public flags and outputs documented and stable.
- Error handling uses clear messages; no secrets in logs.
- I/O is atomic where it writes files.
- Cross-platform paths and encodings handled.

Reviewer checklist:
- API stability and clear semantics.
- Reasonable performance and memory behavior.
- Safe parsing and bounds checks.
- Minimal surface area for new options.
- Adequate comments for tricky code.

## Documentation
Update user-facing docs for any new commands or flags:
- CLI help text
- README examples
- Man page or markdown command docs (if present)
- Shell completion generators

Examples must be copy-paste runnable and safe.

## Performance and Benchmarks
If your change affects hot paths:
- Provide micro or macro benchmarks.
- Compare vs main and include dataset sizes and hardware.
- Avoid premature optimization; keep code readable.

## Security and Responsible Disclosure
Do not file public issues for exploitable bugs. Follow SECURITY.md.  
Vulnerability reports: alanursapu@gmail.com with subject [security][acx].  
Avoid introducing new dependencies without justification and license review.

## Release and Changelog
Changelog follows Keep a Changelog and SemVer:
- Unreleased section accumulates changes.
- Link PR numbers and authors.
- For behavior changes, document migration notes.

Release steps (maintainer):
- Ensure CI is green and SBOM/licenses pass.
- Tag version and update changelog.
- Build and sign artifacts; publish checksums.
- Publish release notes and update package manifests.

## Code of Conduct
We follow the Contributor Covenant. By participating, you agree to uphold a respectful, harassment-free collaboration.

## Attribution
- Primary maintainer: Alan (AUSP59) • alanursapu@gmail.com
- Contributors are listed in the changelog. Please add yourself in your PR.

Thank you for contributing to ACX Contact CLI.