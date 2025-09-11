# Integrity & Reports

## Hashing
```
acx hash [<file>] [--json]
```
Prints a SHA-256 hash of the given file (defaults to `ACX_DATA_FILE` or `acx-data.jsonl`).

## Reports
```
acx report [<file>] [--json]
```
Scans a JSONL store and reports basic stats (record count, unique emails).

## Local Provenance
Use `scripts/provenance_local.py dist` to emit a JSON document with SHA-256 and sizes
for your build artifacts.
