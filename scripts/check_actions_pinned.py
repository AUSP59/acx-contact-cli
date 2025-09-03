#!/usr/bin/env python3
import sys, re, glob, yaml
bad = []
for path in glob.glob('.github/workflows/*.yml'):
    d = yaml.safe_load(open(path))
    for job in (d or {}).get('jobs', {}).values():
        for step in job.get('steps', []):
            uses = step.get('uses')
            if uses and '@' not in uses:
                bad.append((path, uses))
if bad:
    print("Unpinned actions detected:")
    for p,u in bad: print(p, "->", u)
    sys.exit(1)
print("All actions are version-pinned or not using 'uses'.")
