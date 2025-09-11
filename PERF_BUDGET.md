# Performance Budget
- Track `bench.json` from CI (`bench.yml`).
- Define acceptable wall-time drift of +/-10% vs previous main builds.
- Use LTO/PGO selectively (`ACX_LTO=ON`, `ACX_PGO_PHASE=use`) for releases.
