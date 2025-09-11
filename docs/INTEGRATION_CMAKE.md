# Consuming ACX via CMake
```
find_package(acx REQUIRED)
target_link_libraries(your_app PRIVATE acx::acxlib)
```
See `examples/integration/cmake` for a minimal example.
