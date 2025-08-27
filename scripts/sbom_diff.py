#!/usr/bin/env python3
import sys, json
a = json.load(open(sys.argv[1]))
b = json.load(open(sys.argv[2]))
def comps(x): return sorted([c.get('name','')+":"+c.get('version','') for c in x.get('components',[])])
ca, cb = comps(a), comps(b)
only_a = sorted(set(ca) - set(cb))
only_b = sorted(set(cb) - set(ca))
print("Only in A:", only_a)
print("Only in B:", only_b)
exit(0)
