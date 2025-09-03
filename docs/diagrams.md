# Diagrams

```mermaid
flowchart TD
  CLI[acx CLI] -->|commands| Core[Contact Core]
  Core --> Storage[(Local Storage)]
  Core --> Parsers[CSV/JSONL Parsers]
  Core --> Validation[Schema Validation]
  CLI --> Output[JSON / Table]
```
