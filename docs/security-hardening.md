# Security Hardening Guide
- Run container as non-root (already default); add `--read-only` and explicit volumes.
- Use `--cap-drop=ALL` and mount with `:ro` when possible.
- Example seccomp profile in `security/seccomp/acx.json`.
