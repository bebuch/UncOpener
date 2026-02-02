#!/bin/bash
# Run the same tests as CI locally
# Usage: ./tools/run-ci-tests.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

echo "=== Format Check ==="
if ! find src tests -name '*.cpp' -o -name '*.hpp' | xargs clang-format --dry-run --Werror 2>&1; then
    echo "Format check failed. Run: find src tests -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i"
    exit 1
fi
echo "Format check passed."
echo

echo "=== Configure ==="
cmake --preset default
echo

echo "=== Build (Debug) ==="
cmake --build --preset debug
echo

echo "=== Test (Debug) ==="
# QT_QPA_PLATFORM=offscreen is set via CMake's set_tests_properties()
ctest --preset debug
echo

echo "=== Build (Release) ==="
cmake --build --preset release
echo

echo "=== Test (Release) ==="
ctest --preset release
echo

echo "=== Run clang-tidy ==="
find src -name '*.cpp' | xargs clang-tidy -p build
echo

echo "=== All CI checks passed ==="
