# Audit Logging

- Set `ACX_AUDIT_LOG=/path/to/audit.jsonl` to enable append-only JSONL events.
- Events include: `add`, `edit`, `delete`, `import`, `export` with ISO-8601 UTC timestamps.
- Each event may include fields like `id`, `format`, `in`, `out`, and the OS user.
- Rotate or ship logs using your preferred log pipeline. No PII redaction is applied automatically here; combine with policy redaction for exports.
