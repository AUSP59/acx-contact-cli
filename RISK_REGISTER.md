# Risk Register
- Supply chain CVEs in toolchain -> Mitigation: CodeQL, OSV, Trivy, Semgrep; SBOM + alerts
- Regressions in perf -> Mitigation: bench gate with baseline (+10%)
- Release artifact tampering -> Mitigation: provenance SLSA; checksums; (optional) cosign
- Knowledge bus factor -> Mitigation: docs, ADRs/RFCs, MAINTAINERS, runbooks
