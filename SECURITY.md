# Security Policy — ACX Contact CLI

Maintainer: Alan (AUSP59) • alanursapu@gmail.com  
Project: Fast C++20 CLI for processing contact datasets.

## Supported Versions
| Version | Status | Security Fixes |
|--------:|:------:|:---------------|
| main    | stable | yes            |
| latest tagged release | stable | yes |
| < last-1 release | maintenance | high/critical only |
| EOL or forks | unsupported | no |

## Report a Vulnerability
Use **one** of these:
1) Email: **alanursapu@gmail.com** with subject `[security][acx]`.
2) GitHub → **Security advisories (GHSA draft)** on the repo (preferred for encryption-at-rest).

Do **not** open public issues for exploitable bugs.

### Optional Encryption
If you need encryption, request a temporary PGP key via email. As a fallback, send a password-protected ZIP (AES-256) and share the password over a different channel.

## Disclosure and SLA
- Acknowledgement: within **72 hours**.
- Triage + CVSS 3.1 score: within **7 days**.
- Fix or mitigation target after triage:
  - Critical (CVSS ≥ 9.0): **14 days**
  - High (7.0–8.9): **30 days**
  - Medium (4.0–6.9): **60 days**
  - Low (≤ 3.9): **90 days**
- Coordinated release with credit to reporter, unless anonymity is requested.

If a fix needs more time, we will provide interim mitigations and a new ETA.

## What to Include in Your Report
- Affected version/commit and platform.
- Impact and CVSS vector if known.
- Reproducer (inputs, config, dataset size).
- Proof of concept:
    - Minimal CLI commands and sample files.
    - Expected vs actual behavior.
- Any logs or crash outputs (strip secrets).  
Template:

    Title: <short summary>
    Affected: acx <version/commit>, OS <name/version>, CPU/Arch
    Impact: <RCE / DoS / info leak / privilege escalation / data corruption>
    CVSS 3.1: <vector and base score if available>
    Steps:
      1) <command>
      2) <command>
    PoC files:
      - <path/description or gist link>
    Workarounds:
      - <any mitigations discovered>
    Reporter: <name or anonymous>

## Scope
In scope:
- CLI binary `acx` and internal libraries.
- Parsers and writers for CSV/JSONL.
- Optional backends: JSONL, SQLite, RocksDB.
- Optional features: Zstandard, OpenSSL, libphonenumber, OpenTelemetry.
- Installer scripts, container images, packaging manifests, CI workflows for this repo.

Out of scope:
- Vulnerabilities in third-party services or libraries **without** an exploit path through ACX. Report upstream and CC us.
- Social engineering, SPF/DMARC issues, email spoofing.
- Denial-of-service from unrealistic resource limits (e.g., terabyte inputs on tiny machines) unless it bypasses configured limits.
- Debug or non-default builds with sanitizers disabled protections compared to Release.

## Threat Model (high level)
- Inputs: untrusted CSV/JSONL with attacker-controlled content.
- Risks: parser crashes, OOM, path traversal, injection in logs, weak redaction.
- Goals: memory safety, predictable resource bounds, durable I/O, correct PII handling, tamper-evident artifacts.

## Secure Development Practices
- **Compilation hardening (Release)**:
    - Linux/macOS: `-fstack-protector-strong -D_FORTIFY_SOURCE=3 -fPIE -pie -Wl,-z,relro,-z,now`
    - Windows: `/GS /guard:cf /Qspectre /DYNAMICBASE`
- **UB/ASan** in CI for debug.  
- **Static analysis**: clang-tidy, cppcheck, Semgrep, CodeQL.  
- **Fuzzing**: targets for CSV/JSONL; run in CI on seeds + short time budget.  
- **Atomic writes**: temp + fsync + rename.  
- **Constant-time** comparisons for secrets; no secret data in logs.

## Supply Chain and Releases
- **Pinned dependencies** via package manager or vcpkg manifests.
- **SBOM** generation (SPDX/JSON) shipped with releases.
- **License scan** and policy gate before publish.
- **Reproducible build** target documented.
- **Artifacts**: SHA-256 checksums and detached signatures.
- **Signing**: release archives and container images, e.g. with cosign.
- **Provenance**: SLSA-style build metadata when available.

Example verification:

    curl -LO https://example/releases/acx.tar.gz
    curl -LO https://example/releases/acx.tar.gz.sig
    sha256sum -c acx.tar.gz.sha256
    cosign verify-blob --signature acx.tar.gz.sig acx.tar.gz

## Runtime Hardening and Configuration
- Default: no network access; telemetry is opt-in.
- Use least-privilege file permissions for input/output directories.
- Prefer local SSD for tmp; set `ACX_TMPDIR`.
- For huge datasets, set CPU/memory quotas via your scheduler.

## Secrets and PII
- Do not feed real secrets to examples or issue templates.
- Use irreversible wipe for PII when compliance requires it.
- Redaction applies to error paths and logs.

## Handling Reports About Third-Party Dependencies
Primary dependencies include (non-exhaustive): SQLite, RocksDB, Zstandard, OpenSSL, libphonenumber, fmt, spdlog.  
If your finding is exclusively upstream, please:
1) Report to the upstream project first.  
2) Open a private advisory to us with cross-references so we can:
    - Pin to a safe version or apply a patch.
    - Publish a coordinated advisory if ACX is affected.

## Backport and Patch Policy
- Main receives the fix first.
- Latest release gets a patch release.
- Previous release gets backports if severity is High or Critical and patching risk is low.

## Public Advisory Workflow
- We publish a GitHub Security Advisory with GHSA identifier.
- Changelog entry lists CVE/GHSA, severity, and upgrade guidance.
- Coordinated timing with downstream packagers when applicable.

## Verification Checklist (Users)
Before running on sensitive data:
- Use the latest signed release. Verify checksum and signature.
- Run `acx selfcheck` to verify environment assumptions.
- Validate on a small sample first.
- Enable Zstd only if both read/write sides support it.
- Confirm redaction mode reflects your policy.

Example quick smoke test:

    acx --version
    acx help --json
    acx selfcheck

## Responsible Research and Safe Harbor
We support good-faith research and coordinated disclosure. If you comply with this policy and applicable laws, we will not initiate legal action for your research on this project and its official artifacts.

## Credit
We maintain a Hall of Fame in the changelog. Provide the display name, link, and whether you prefer anonymity.

## Contact
Security reports: **alanursapu@gmail.com**  
GitHub: **AUSP59**

Last updated: 2025-09-10