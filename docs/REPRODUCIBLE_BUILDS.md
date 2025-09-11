# Reproducible Builds

Set `SOURCE_DATE_EPOCH` to the last commit timestamp before building:
```bash
export SOURCE_DATE_EPOCH=$(git log -1 --pretty=%ct)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
The build defines `ACX_SOURCE_DATE_EPOCH` when present, enabling stable stamping in logs/help.
