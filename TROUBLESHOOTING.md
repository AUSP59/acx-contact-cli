# Troubleshooting
- **Build fails:** run `bash scripts/preflight.sh` to verify toolchain.
- **Colors not readable:** use `--no-color` or set `NO_COLOR=1`.
- **Automation output:** prefer `--json` when supported by a command.
- **Permission errors in containers:** run as non-root and mount a writable volume for outputs.
