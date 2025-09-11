# Build options

- `ACX_WERROR` (ON): treat warnings as errors.
- `ACX_LTO` (OFF): enable interprocedural optimization (LTO) when supported.
- `ACX_PGO` (OFF): simple PGO; use `-DACX_PGO_PHASE=generate|use`.
- `ACX_WITH_OPENSSL` (ON): use OpenSSL for HMAC/SHA-256 when available.
- `ACX_WITH_LIBPHONENUMBER` (OFF): full E.164 parsing/formatting.
- `ACX_WITH_FUZZ` (OFF): build libFuzzer harnesses (`fuzz/`).

