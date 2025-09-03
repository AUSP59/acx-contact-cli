# Release Process
- CI green (build-matrix, sanitizers, coverage, clang-tidy, codeql, gitleaks, scorecard)
- Bump version, update CHANGELOG
- Tag `vX.Y.Z`
- Packages workflow uploads artifacts (zip/tgz/deb/rpm)
- Generate SBOM (`scripts/make_sbom.sh`), attach to release
- Optionally sign binaries with cosign (instructions in SECURITY.md)
