# Policy for Redaction

Minimal JSON file:
```json
{ "redact_name": false, "redact_email": true, "redact_phone": true }
```
CLI uses it on `export` automatically if provided in `acx.conf` or via `--policy`.
