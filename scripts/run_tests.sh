#!/usr/bin/env bash
# ==============================================================================
# run_tests.sh - Headless Master Test Harness Runner for Penguin Media Player
# ==============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${PROJECT_ROOT}"

export PYTHONPATH="${PROJECT_ROOT}:${PYTHONPATH:-}"
export QT_QPA_PLATFORM="offscreen"
export XDG_RUNTIME_DIR="/tmp/penguin_runtime_test_${UID:-1000}_$$"
mkdir -p "${XDG_RUNTIME_DIR}"
chmod 700 "${XDG_RUNTIME_DIR}"

cleanup() {
    rm -rf "${XDG_RUNTIME_DIR}"
}
trap cleanup EXIT INT TERM

echo ">>> Initializing Penguin Headless Test Environment..."
echo ">>> Working Directory: ${PROJECT_ROOT}"
echo ">>> QT_QPA_PLATFORM:   ${QT_QPA_PLATFORM}"
echo ">>> XDG_RUNTIME_DIR:   ${XDG_RUNTIME_DIR}"

if command -v dbus-run-session &>/dev/null; then
    echo ">>> Running with isolated D-Bus session (dbus-run-session)..."
    dbus-run-session python3 tests/test_e2e_runner.py "$@"
else
    echo ">>> Running in direct headless mode..."
    python3 tests/test_e2e_runner.py "$@"
fi
