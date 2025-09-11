# Configuration

ACX reads configuration (JSON) from the first file found in:
1. `ACX_CONFIG` environment variable (absolute path)
2. `$XDG_CONFIG_HOME/acx/config.json`
3. `~/.config/acx/config.json`

## Recognized keys
- `data_file` (string): path to contacts store (JSONL).
- `backup_dir` (string): directory to store backups.
- `audit_log` (string): path for JSONL audit events.
- `lock_dir` (string): directory used to place advisory lock files.
- `region` (string): default country code (e.g., `MX`) for phone normalization.
- `output` (string): default output format: `human` or `json`.

See `examples/config.json` for a full example.
