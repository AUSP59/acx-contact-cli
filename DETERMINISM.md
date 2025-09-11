# Determinism

- Export then re-import must not change dataset when dedup rules are stable.
- `acx determinism-check` automates this property.
- Non-determinism sources mitigated: stable sort orders, ISO-8601 timestamps only when creating new contacts.
