#!/usr/bin/env python3
import json, hashlib, sys, os, time
paths = sys.argv[1:] or ['dist']
records = []
def sha256(p):
    h = hashlib.sha256()
    with open(p, 'rb') as f:
        for ch in iter(lambda: f.read(1<<20), b''):
            h.update(ch)
    return h.hexdigest()

for base in paths:
    if os.path.isdir(base):
        for root,_,files in os.walk(base):
            for f in files:
                p = os.path.join(root,f)
                records.append({"path": p, "sha256": sha256(p), "size": os.path.getsize(p)})
    elif os.path.isfile(base):
        records.append({"path": base, "sha256": sha256(base), "size": os.path.getsize(base)})
att = {"type":"local-provenance","generated_at": int(time.time()), "artifacts": records}
print(json.dumps(att, indent=2))
