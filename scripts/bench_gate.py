#!/usr/bin/env python3
import json, sys, math
baseline = json.load(open("benchmarks/baseline.json"))
current  = json.load(open("bench.json"))
b = baseline["bench_thread_pool"]["wall_seconds"]
c = current["bench_thread_pool"]["wall_seconds"]
if b <= 0:
  print("No baseline set; recording current (gate PASS).")
  sys.exit(0)
limit = b * 1.10
print(f"baseline={b:.6f}s current={c:.6f}s limit={limit:.6f}s")
sys.exit(0 if c <= limit else 1)
