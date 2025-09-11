# Symbols & Debugging

- Linux: produce DWARF symbols in Debug, strip in Release as needed.
- macOS: generate `dSYM` via `-g` and `dsymutil` (CI can archive as artifact).
- Windows: generate `.pdb` with MSVC; keep for crash triage.
