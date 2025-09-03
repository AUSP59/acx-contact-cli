# Configuration

File: `acx.conf` (INI-like), and environment variables override.

```ini
default_country=MX
policy_file=policy.json
sqlite_pragmas=journal_mode=WAL;synchronous=NORMAL
```

Env overrides:
- `ACX_DEFAULT_COUNTRY`, `ACX_POLICY_FILE`, `ACX_SQLITE_PRAGMAS`
