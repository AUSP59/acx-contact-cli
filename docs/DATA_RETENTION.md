# Data Retention Policy (Project-level)

- No telemetry or remote transfer by default.
- Local audit log (if `ACX_AUDIT_LOG` is set) is **append-only** JSONL; rotate externally.
- Backups/exports are user-triggered; signatures (HMAC) are supported for integrity.
- Recommended default retention for audit logs: 90 days, configurable by operator.
