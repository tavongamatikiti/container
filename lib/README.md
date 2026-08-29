# lib/

Vendored third-party libraries.

Drop each dep in `lib/<name>/` and wire in `../CMakeLists.txt`:

```cmake
add_subdirectory(lib/log.c)
target_link_libraries(container PRIVATE log.c)
```

Examples from blog: `argtable`, `log.c` (https://github.com/rxi/log.c).

Leave empty if using system packages (`find_package`).
