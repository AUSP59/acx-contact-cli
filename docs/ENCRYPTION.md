# File Encryption (AES-256-GCM)

- Commands: `acx encrypt --in F --out F [--aad STR]`, `acx decrypt --in F --out F`.
- Keys: set `ACX_ENC_KEY` (hex, 32 bytes => 64 hex chars) **or** `ACX_ENC_KEY_FILE` (hex).
- Format: custom header `ACXEG1` + IV(12) + TAG(16) + ciphertext.
- Integrity: AES-GCM authenticates contents and AAD.
- Keygen: `acx keygen --bytes 32` prints a random key (hex).
