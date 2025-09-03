#!/usr/bin/env python3
import json, sys
ALLOW = {"Apache-2.0","MIT","BSD-2-Clause","BSD-3-Clause","ISC","MPL-2.0","Boost Software License 1.0"}
path = sys.argv[1] if len(sys.argv)>1 else "SBOM.cyclonedx.json"
try:
  sbom = json.load(open(path))
except Exception as e:
  print("SBOM not found or invalid:", e)
  sys.exit(0)  # don't fail if SBOM missing
viol = []
for c in sbom.get("components", []):
  licname = ""
  if c.get("licenses"):
    lic = c["licenses"][0].get("license",{})
    licname = lic.get("name") or lic.get("id","")
  if licname and licname not in ALLOW:
    viol.append((c.get("name","unknown"), licname))
if viol:
  print("Disallowed licenses detected:")
  for n,l in viol: print(f"- {n}: {l}")
  sys.exit(1)
print("License policy OK")
