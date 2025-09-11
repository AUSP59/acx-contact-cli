# Architecture

Modules:
- `model` — domain types (Contact, ContactBook)
- `storage` — JSONL storage, backups, atomic writes
- `validation` — input validation and normalization
- `cli` — command-line argument parsing and dispatch
- `utils` — time, uuid, hashing, file ops
- `audit` — repository self-audit

Design principles:
- Deterministic outputs
- Clear ownership and small modules
- No external dependencies
