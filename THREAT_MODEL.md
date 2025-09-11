# Threat Model (CLI scope)

**Scope**: Local CLI using JSONL/SQLite/RocksDB engines. No network services.  
**Assets**: contact data files, backups, audit logs, SBOM, release artifacts.  
**Adversaries**: local user error, untrusted input files, tampered artifacts.  
**Controls**:
- Input validation, RFC4180 parser, unique constraints (SQLite), duplicate prevention.
- HMAC-signed backups; optional zstd compression; `verify-file` (sha256).
- Repo lock + atomic writes; journal for JSONL.
- CI: CodeQL, Gitleaks, fuzz smoke, coverage, hardened flags (RELRO/NOW, SSP, FORTIFY).
- Release: SBOM, checksums, cosign signing (keyless).  
**Out of scope**: remote attackers, key management, enterprise KMS/TPM, GUI/API.
