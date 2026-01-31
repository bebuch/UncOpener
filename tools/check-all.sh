#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

PRESET="${1:-dev}"

echo "=== Running all checks with preset: $PRESET ==="
echo

echo "--- Configuring ---"
cmake --preset "$PRESET"
echo

echo "--- Checking format ---"
cmake --build --preset "$PRESET" --target format-check
echo

echo "--- Building ---"
cmake --build --preset "$PRESET"
echo

echo "--- Running tests ---"
ctest --preset "$PRESET"
echo

echo "=== All checks passed! ==="
