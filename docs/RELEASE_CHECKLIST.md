# Release Checklist
- [ ] Update CHANGELOG (release-please)
- [ ] Tag `vX.Y.Z`
- [ ] CI: build matrices green, SBOM & provenance generated
- [ ] Run `make dist` and verify checksums
- [ ] (Optional) Sign artifacts with cosign
- [ ] Publish docs site & API docs artifacts
