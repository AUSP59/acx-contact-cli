# Advanced Data Operations

- `normalize [<in>] [<out>]` — trims names, lowercases emails, strips phone to digits (keeps leading `+`).
- `merge <out> <file1> <file2> [...]` — merges files removing duplicates by email.
- `sample [<in>] [<out>] [N]` — reservoir sampling of N records (default 100).
- `grep [<in>] [<out>] <regex>` — filters by regex on `name` or `email` (case-insensitive).
- `checksum <file> <expected|.sha256>` — verifies file integrity against a hex value or a `.sha256` file.
