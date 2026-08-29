# container

C project scaffold following [Luca Vallin – How to Structure C Projects](https://www.lucavallin.com/blog/how-to-structure-c-projects-my-experience-best-practices) (flat layout), adapted for **CLion + CMake** (no `.vscode`).

```
container/
├── bin/              # executable (CMAKE_RUNTIME_OUTPUT_DIRECTORY) – gitignored
├── build/            # out-of-source builds – gitignored (use cmake-build-debug/)
├── cmake-build-*/    # CLion build dirs – gitignored
├── docs/             # documentation
├── include/container/     # public headers (*.h) – API
│   ├── container.h
│   └── version.h
├── lib/              # vendored third-party libs (add via add_subdirectory)
├── scripts/          # setup / helper scripts
├── src/              # implementation (*.c)
│   ├── main.c        # CLI entry
│   └── container.c
├── tests/            # tests (CTest, no CUnit required)
│   └── test_container.c
├── .clang-format
├── .clang-tidy
├── .gitignore
├── CMakeLists.txt    # CLion entry point (C23, sanitizers in Debug)
└── README.md
```

## Build (CLion)

Open `~/CLionProjects/container` in CLion → it auto-configures CMake (Debug/Release). Or CLI:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./bin/container
# or
./build/test_container
ctest --test-dir build --output-on-failure
cmake --build build --target check   # alias
```

`compile_commands.json` is generated (`CMAKE_EXPORT_COMPILE_COMMANDS=ON`) → CLion + clangd use it automatically. Symlink if needed:

```sh
ln -sf build/compile_commands.json .
```

## Add a new module

1. `include/container/foo.h` – public API
2. `src/foo.c` – `#include "container/foo.h"`
3. `CMakeLists.txt`: add `src/foo.c` to `add_library(container ...)`
4. `tests/test_foo.c` → `add_executable` + `add_test` (copy `test_container` block)

Keep headers and sources 1:1, keep files small/focused (Vallin’s “concerns”).

## Formatting / Linting

```sh
cmake --build build --target format   # clang-format -i
clang-tidy src/*.c -- -Iinclude       # via .clang-tidy
```

Nvim already has `clangd` + `clang-format` 21.1.8 wired (`conform.nvim`).

## lib/

Drop vendored code in `lib/<name>/` and add in `CMakeLists.txt`:

```cmake
add_subdirectory(lib/log.c)
target_link_libraries(container PRIVATE log.c)
```

Or use system packages: `find_package(PkgConfig)` etc.

## Tests

No mandatory CUnit – `tests/test_container.c` uses `<assert.h>` + CTest (simple, Vallin notes CUnit setup was unused in practice). Swap to [Unity](https://github.com/ThrowTheSwitch/Unity) / [CUnit](https://cunit.sourceforge.net/) if needed:

```sh
# CUnit example
brew install cunit
# CMakeLists: find_package(CUnit) + target_link_libraries(test_container PRIVATE CUnit)
```

## Scripts

`scripts/` is for `setup.sh`, `run.sh` etc. Example `scripts/setup.sh`:

```sh
#!/bin/sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
```

## Notes vs. blog

- Kept `bin/` + `build/` + `include/` + `lib/` + `src/` + `tests/` + `docs/` from blog.
- Dropped `.vscode/` (per request), `.devcontainer/` + `.github/` (add when needed).
- Replaced `Makefile` with `CMakeLists.txt` for CLion; Makefile can be re-added if you want `make` parity.
- `lib/` stays empty (vendored `argtable`/`log.c` example in blog) – gitkeeps not needed.
