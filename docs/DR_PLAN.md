# Disaster Recovery Plan
- Source of truth: Git repository; nightly backups of default branch and tags.
- Release artifacts: regenerate from tagged source; verify via SBOM/provenance and checksums.
- Docs site: rebuild from main via workflow; keep static site artifact in Releases.
