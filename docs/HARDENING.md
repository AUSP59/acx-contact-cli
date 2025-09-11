# Hardening Guide

- Compile-time: `-fstack-protector-strong`, `-D_FORTIFY_SOURCE=2`, RELRO+NOW, sanitizers in CI.
- Reproducible builds: set `SOURCE_DATE_EPOCH` in CI; use Nix flake for deterministic toolchains.
- Crypto: prefer OpenSSL backend; fallback is hermetic; constant-time comparisons used.
