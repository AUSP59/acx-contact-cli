#!/usr/bin/env python3
import sys, os, re
ROOT = os.getcwd()
ok = True
for root, _, files in os.walk(ROOT):
    if any(x in root for x in [".git", "build", "dist", ".venv"]): continue
    for f in files:
        if f.endswith((".cpp",".hpp",".c",".h",".py",".sh",".cmake","CMakeLists.txt")):
            p = os.path.join(root,f)
            try:
                with open(p, "r", encoding="utf-8", errors="ignore") as fh:
                    head = fh.read(200)
                if "SPDX-License-Identifier:" not in head and "SPDX-License" not in head and "Apache-2.0" not in head:
                    print("Missing SPDX hint:", p)
                    ok = False
            except Exception:
                pass
print("SPDX check", "OK" if ok else "FAILED")
sys.exit(0 if ok else 1)
