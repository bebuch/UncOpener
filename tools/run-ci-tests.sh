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

echo "=== Configure (Debug) ==="
cmake --preset dev
echo

echo "=== Build ==="
cmake --build --preset dev -j$(nproc)
echo

echo "=== Run Tests ==="
# QT_QPA_PLATFORM=offscreen is set via CMake's set_tests_properties()
ctest --preset dev --output-on-failure
echo

echo "=== Run clang-tidy ==="
find src -name '*.cpp' | xargs clang-tidy -p build/dev
echo

echo "=== All CI checks passed ==="
