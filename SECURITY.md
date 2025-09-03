# Security Policy — ACX Enterprise Contact CLI

**Last Updated:** 2025-08-27

We take security seriously and appreciate coordinated, responsible disclosure. This document explains how to report vulnerabilities and what you can expect from us.

---

## Supported Versions

We provide security fixes for the latest **minor** release line.

| Version Line | Status        |
|--------------|---------------|
| `vX.Y.*`     | **Supported** |
| older        | Best-effort   |

> We recommend upgrading to the latest release before filing reports to confirm the issue persists.

---

## Reporting a Vulnerability

Please report security issues **privately** via email:

- **Security Team:** `security@acx.dev`

> Do **not** open a public GitHub issue for sensitive reports.

If you require encryption, request our PGP key at the address above. We’ll arrange an encrypted channel.

### What to Include

Please provide as much detail as possible:

- A clear description of the issue and **impact**.
- **Steps to reproduce**, sample data/files, and minimal PoC if applicable.
- Affected **version/commit**, OS, compiler, and build flags.
- Any **workarounds** you discovered.
- Your desired **credit** name/handle (optional).

---

## Our Response Process & SLA

- **Acknowledgement:** within **72 hours**.
- **Initial assessment & triage:** within **7 days** (severity, scope, repro).
- **Fix targets (guideline):**
  - **Critical:** 7 days
  - **High:** 14 days
  - **Medium:** 30 days
  - **Low:** 90 days

We may adjust timelines for complexity or dependency coordination. We will keep you updated at key milestones and offer coordinated disclosure.

---

## Coordinated Disclosure

By default, we follow a **90-day** coordinated disclosure window starting from acknowledgement. We’ll:
1. Develop and test a fix.
2. Release patched versions and advisories (GitHub Security Advisory / GHSA).
3. Credit the reporter (unless anonymity is requested).

If exploitation is in the wild or risk is extreme, we may accelerate disclosure.

---

## Safe Harbor for Security Research

We commit to not pursue or support legal action against researchers for **good-faith** findings that:
- Respect privacy and **only** use data you own or have permission to test with.
- Avoid service disruption (no DoS, no stress tests on shared infra).
- Do not exfiltrate or destroy data.
- Do not access other users’ systems or data.

If you are unsure whether your testing is in scope, ask us first at `security@acx.dev`.

---

## Scope & Out-of-Scope

**In scope**
- The CLI binary `acx` and its C++ source under this repository.
- CMake build scripts and configuration.
- Provided container artifacts (Dockerfiles) built from this repo.

**Out of scope**
- Third-party services, package registries, or OS-level vulnerabilities.
- Issues only present when the software is misconfigured against documented guidance.
- Social engineering, physical attacks, spam.
- Denial-of-service via unreasonable inputs/volumes beyond documented limits.

For third-party dependency issues, please also notify the upstream project. We will track and, where feasible, pin or patch.

---

## Hardening & Defense-in-Depth (Implemented)

The project includes practical security measures you can verify:

- **Memory-safety checks (development):** AddressSanitizer / UndefinedBehaviorSanitizer / ThreadSanitizer (CMake toggles).
- **Reproducible builds:** honors `SOURCE_DATE_EPOCH`.
- **Atomic writes:** temporary file + `rename()` to prevent partial-write corruption.
- **Determinism & seeding:** `--deterministic` + `--seed` for reproducible operations.
- **Integrity tools:** `hash`, `checksum`, and `scripts/provenance_local.py`.
- **Containers:** standard and distroless Dockerfiles with `HEALTHCHECK`.
- **CI hygiene:** concurrency, fork-safe skips, and non-blocking heavy analyzers.

> Optional features such as **artifact signing/verification** (e.g., Sigstore/cosign) are **not enabled by default** and require organization-level setup. We can document integration if you enable it in your environment.

---

## Reporting Template (Email)

Subject: [SECURITY] <short description>

Product: ACX Enterprise Contact CLI
Version/Commit: vX.Y.Z (commit <sha>)
Environment: <OS, compiler, build flags>

Summary:
<What is the issue? What can an attacker achieve?>

Steps to Reproduce:
<Exact steps / commands / inputs>

Impact:
<Confidentiality / Integrity / Availability impact, data scope>

Workarounds:
<Any mitigations you found>

Reporter:
<Name or handle as you would like it credited>

yaml
Copiar código

---

## Handling Secrets

- Do **not** submit secrets in tickets, PRs, logs, or test artifacts.
- If you accidentally commit a secret, **revoke/rotate** it immediately and contact us.
- The repository and CI pipelines are designed to **avoid requiring secrets** for core build/test.

---

## Vulnerability Classification

We use **CVSS v3.1** guidelines to categorize severity (Critical/High/Medium/Low) and decide patch priority. Where appropriate, we will request GHSA IDs and, if applicable, CVE assignments via GitHub’s advisory process.

---

## Secure Development Guidelines (For Contributors)

- Include `SPDX-License-Identifier` headers (Apache-2.0) in all new files.
- Prefer **bounds-checked** operations; avoid raw pointer arithmetic when not necessary.
- Validate **all** user inputs; reject malformed JSONL records gracefully.
- Keep **`--json`** outputs stable and machine-parseable (one JSON per line).
- Use **atomic file writes** for any file-modifying command.
- Add tests for both success and error paths; run sanitizers for new code paths.
- Never add telemetry or network calls without explicit documentation and a flag.

See `CONTRIBUTING.md` for full contributor requirements.

---

## Contact

- **Security Team:** `security@acx.dev`  
- **Emergency (coordinated disclosure):** `security@acx.dev`  
- **Code of Conduct:** `conduct@acx.dev`

We appreciate your help in keeping ACX secure for everyone.