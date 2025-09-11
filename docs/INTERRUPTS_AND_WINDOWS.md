# Interrupts & Windows Console

- Graceful handling of `SIGINT`/`SIGTERM` ensures clean shutdown paths.
- On Windows, the CLI now enables **UTF-8 output** and **VT sequences** for ANSI-style formatting.
- Colors can always be disabled via `--no-color` or `NO_COLOR=1`.
