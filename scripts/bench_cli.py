#!/usr/bin/env python3
import time, subprocess, json, sys
CMDS = [
  ["./build/acx","report","examples/contacts.jsonl","--json"],
  ["./build/acx","validate","examples/contacts.jsonl","--json"],
]
thr = int(sys.argv[1]) if len(sys.argv)>1 else 0
env = dict(**dict(os.environ))
if thr: env["ACX_THREADS"]=str(thr)
def run(cmd):
  t0=time.time()
  out=subprocess.check_output(cmd, env=env)
  dt=time.time()-t0
  try: j=json.loads(out.decode()); j["_elapsed_s"]=dt; return j
  except: return {"cmd":" ".join(cmd), "_elapsed_s": dt}
res=[run(c) for c in CMDS]
print(json.dumps(res, indent=2))
