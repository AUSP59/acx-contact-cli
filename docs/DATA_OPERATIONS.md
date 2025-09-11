# Data Operations (CLI)

- `validate [<file>] [--json]` — fast validator: requires `name` non-empty; if `email` exists, it must look valid.
- `dedupe [<file>] [<out>]` — remove duplicates by `email` (fallback: full line). Writes to `<out>` or `acx-dedupe.jsonl`.
- `backup [<file>] [<dest-dir>]` — copies the store to a timestamped file under `<dest-dir>` (defaults to `backups/`).
- `diff <A> <B> [--json]` — set-diff by `email`; prints counts (added/removed).
- `redact [<in>] [<out>]` — masks emails and digits in `phone` for safe sharing.
