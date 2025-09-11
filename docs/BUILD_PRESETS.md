# Build Presets
- **Default**: Release, hardened.
- **Tiny**: `-DACX_TINY=ON` for minimum size (no RTTI/exceptions in safe areas).
- **Static (Linux)**: `-DACX_STATIC=ON` to attempt static linking where toolchain allows.
