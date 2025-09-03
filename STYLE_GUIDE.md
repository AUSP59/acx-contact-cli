# C++ Style Guide (ACX)

- C++20, RAII, `std::unique_ptr`/`std::shared_ptr` as needed.
- No raw new/delete; avoid exceptions in hot paths.
- Prefer `std::optional` to sentinel values.
- Error handling: explicit `StatusOr<T>`-style or error codes.
