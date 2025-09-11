# Security Review Checklist
- [ ] Inputs sanitized and validated (paths, formats)
- [ ] No dangerous default permissions
- [ ] Minimal container privileges (rootless, read-only fs, seccomp)
- [ ] SBOM updated; scans clean (CodeQL, Semgrep, Trivy, OSV)
- [ ] Provenance generated for releases
