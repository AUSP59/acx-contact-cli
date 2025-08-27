# Compiler & Linker Hardening Flags

Enabled by default (can be toggled in CMake):
- **GCC/Clang**: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion`
- **Security**: `-fstack-protector-strong -D_FORTIFY_SOURCE=3 -fPIC`, link: `-Wl,-z,relro -Wl,-z,now [-pie]`
- **Windows (MSVC)**: `/guard:cf /Qspectre /sdl` + link: `/CETCOMPAT /DYNAMICBASE /NXCOMPAT`

Use `-DACX_WERROR=ON` to enforce zero warnings.
