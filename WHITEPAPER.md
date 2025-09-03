# ACX — Contact CLI Whitepaper

**Goal.** Provide an offline, audit-ready contact management CLI with deterministic behavior, strong security posture, and verifiable provenance — no external services.

## Architecture (High level)
```text
+--------------------+      +-------------------+
|      CLI (acx)     | ---> |      acxlib       |
|  parse, commands   |      |  storage / logic  |
+--------------------+      +-------------------+
             |                         |
             v                         v
       JSONL/SQLite             Index/Verify/HMAC
             |                         |
             +-------- Backup ---------+
```

## Non-Goals
- No network usage by default; no telemetry.
- GUI/server are separate projects.
