#!/usr/bin/env python3
import re, subprocess, sys
bin = sys.argv[1] if len(sys.argv) > 1 else "./build/acx"
out = subprocess.check_output([bin, "--version"], text=True, stderr=subprocess.STDOUT).strip()
if not re.search(r"\b\d+\.\d+\.\d+\b", out):
  print("Version is not semver-like:", out)
  sys.exit(1)
print("Version OK:", out)
