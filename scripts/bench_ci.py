#!/usr/bin/env python3
import json, subprocess, sys, time
def run(cmd):
  t0 = time.time()
  out = subprocess.check_output(cmd, text=True)
  t1 = time.time()
  return {"cmd": cmd, "stdout": out, "wall_seconds": t1-t0}
if __name__ == "__main__":
  res = {}
  res["bench_thread_pool"] = run(["./build/bench_thread_pool"])
  print(json.dumps(res, indent=2))
