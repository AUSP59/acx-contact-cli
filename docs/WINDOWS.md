# Windows build notes

- Use MSVC or ClangCL with CMake + Ninja.
- Optional dependencies via vcpkg: `vcpkg install openssl libphonenumber`.
- Run: `cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DACX_WITH_OPENSSL=ON -DACX_WITH_LIBPHONENUMBER=ON`.
