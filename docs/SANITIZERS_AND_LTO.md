# Sanitizers, LTO & Coverage

```bash
# AddressSanitizer + UBSan
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=RelWithDebInfo -DACX_ASAN=ON -DACX_UBSAN=ON
cmake --build build-asan

# ThreadSanitizer
cmake -S . -B build-tsan -DCMAKE_BUILD_TYPE=RelWithDebInfo -DACX_TSAN=ON

# Link-Time Optimization
cmake -S . -B build-lto -DCMAKE_BUILD_TYPE=Release -DACX_LTO=ON

# Coverage (GCC/Clang)
cmake -S . -B build-cov -DCMAKE_BUILD_TYPE=Debug -DACX_COVERAGE=ON
```
