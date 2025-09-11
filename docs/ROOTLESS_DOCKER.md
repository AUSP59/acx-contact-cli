# Run Rootless Docker

- Build: `docker build -t acx:latest .`
- Run: `docker run --rm --read-only --user 1000:1000 --cap-drop=ALL acx:latest help`
- Apply `security/seccomp/acx.json` for syscall filter.
