# TODO - CMake resume improvements

- [ ] Update `CMakeLists.txt` to build the actual DPI engine (likely `src/dpi_mt.cpp`) rather than the placeholder 3 sources.
- [ ] Replace `include_directories(...)` with `target_include_directories(packet_analyzer ...)`.
- [ ] Add better build defaults: Release fallback, warnings for GCC/Clang, and sensible compile options.
- [ ] Link pthreads on non-Windows via `find_package(Threads)` and `Threads::Threads`.
- [ ] Set a clean runtime output directory (e.g., `${CMAKE_BINARY_DIR}/bin`).
- [ ] Keep `CMAKE_EXPORT_COMPILE_COMMANDS ON`.
- [ ] Build-test using CMake to ensure it compiles.

