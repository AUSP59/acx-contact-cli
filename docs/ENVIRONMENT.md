
# Environment Variables

- `ACX_ENGINE` = jsonl|sqlite (select storage engine)
- `ACX_WITH_OPENSSL` (CMake option) selects OpenSSL at build; runtime uses compiled backend
- `ACX_ZSTD` = 1 to enable zstd compression for backups (if built)
- `ACX_HMAC_KEY` = HMAC key to sign backups
- `ACX_MAX_CONTACTS` = safety limit before refusing new inserts
- `ACX_DEFAULT_COUNTRY`, `ACX_POLICY_FILE`, `ACX_SQLITE_PRAGMAS` see CONFIGURATION.md
- `ACX_LOG_FORMAT` = json|text
- `ACX_LOG_LEVEL` = error|warn|info|debug|trace
- `SOURCE_DATE_EPOCH` respected for reproducible builds (via CMake timestamp injection)
