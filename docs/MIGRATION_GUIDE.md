# Migration Guide
- Exit codes standardized in `EXIT_CODES.md`.
- JSON and no-color flags available; scripts should use `--json` where provided.
- Package consumers: prefer `find_package(acx)`; fallback to `pkg-config acx`.
