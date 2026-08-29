#!/bin/sh
set -e
# Simple setup – mirrors blog's scripts/ intent but for CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ln -sf build/compile_commands.json compile_commands.json
echo "Build done. Run ./bin/container or ctest --test-dir build"
