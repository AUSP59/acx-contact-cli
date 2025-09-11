# Release Policy

- **SemVer**: MAJOR.MINOR.PATCH.
- **Main** is always releasable; tag `vX.Y.Z` to cut a release.
- Each release ships: artifacts (CPack), SBOM, provenance (SLSA), checksums.
- Critical CVE? Patch release with backport per `BACKPORTS.md`.
