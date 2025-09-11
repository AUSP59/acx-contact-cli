# Performance Tuning

- Use `--threads N` (or `ACX_THREADS`) to parallelize `validate` and `report`.
- Default threads: `hardware_concurrency()` or `2` if unknown.
- Use `scripts/bench_cli.py [N]` to measure elapsed times with/without threads.
