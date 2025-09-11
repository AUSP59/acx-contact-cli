# Audit Trail

- Every mutation writes an append-only CSV audit log.
- Backups include HMAC signatures. Verify with `acx verify-backup`.
