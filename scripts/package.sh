#!/usr/bin/env bash
# ==============================================================================
# package.sh - Desktop packaging and installation script for Penguin
# ==============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

PREFIX="${1:-${HOME}/.local}"

echo ">>> Installing Penguin to ${PREFIX}..."

mkdir -p "${PREFIX}/bin"
mkdir -p "${PREFIX}/share/applications"
mkdir -p "${PREFIX}/share/icons/hicolor/scalable/apps"
mkdir -p "${PREFIX}/share/icons/hicolor/48x48/apps"
mkdir -p "${PREFIX}/share/icons/hicolor/256x256/apps"
mkdir -p "${PREFIX}/share/icons/hicolor/512x512/apps"
mkdir -p "${PREFIX}/share/icons/hicolor/symbolic/apps"

install -m 755 "${PROJECT_ROOT}/penguin" "${PREFIX}/bin/penguin"
install -m 644 "${PROJECT_ROOT}/penguin.desktop" "${PREFIX}/share/applications/penguin.desktop"
install -m 644 "${PROJECT_ROOT}/icons/hicolor/scalable/apps/penguin.svg" "${PREFIX}/share/icons/hicolor/scalable/apps/penguin.svg"
install -m 644 "${PROJECT_ROOT}/icons/hicolor/48x48/apps/penguin.png" "${PREFIX}/share/icons/hicolor/48x48/apps/penguin.png"
install -m 644 "${PROJECT_ROOT}/icons/hicolor/256x256/apps/penguin.png" "${PREFIX}/share/icons/hicolor/256x256/apps/penguin.png"
install -m 644 "${PROJECT_ROOT}/icons/hicolor/512x512/apps/penguin.png" "${PREFIX}/share/icons/hicolor/512x512/apps/penguin.png"
install -m 644 "${PROJECT_ROOT}/icons/hicolor/symbolic/apps/penguin-symbolic.svg" "${PREFIX}/share/icons/hicolor/symbolic/apps/penguin-symbolic.svg"

echo ">>> Installation complete."
