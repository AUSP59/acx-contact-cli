#!/usr/bin/env python3
import json,sys
sbom = json.load(open('SBOM.cyclonedx.json')) if len(sys.argv)<2 else json.load(open(sys.argv[1]))
comps = sbom.get('components', [])
print("# Third-Party Notices")
for c in comps:
  name = c.get('name','unknown')
  ver = c.get('version','')
  lic = ""
  if 'licenses' in c and c['licenses']:
    l = c['licenses'][0].get('license',{}).get('name','')
    lic = f" — {l}" if l else ""
  print(f"- {name} {ver}{lic}")
