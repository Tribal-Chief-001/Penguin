#!/usr/bin/env bash
# ==============================================================================
# build.sh - Build script for Penguin Media Player and Test Suites
# ==============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${PROJECT_ROOT}"

echo ">>> Building Penguin Application binary..."
qmake6 penguin.pro
make -j"$(nproc)"

echo ">>> Building Milestone 3 & 4 Test Suite..."
qmake6 tests/test_m3_m4.pro -o tests/Makefile.test_m3_m4
make -C tests -f Makefile.test_m3_m4 -j"$(nproc)"

echo ">>> Build completed successfully."
