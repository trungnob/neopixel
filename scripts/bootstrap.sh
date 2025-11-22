#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
VENV_DIR="${VENV_DIR:-$PROJECT_ROOT/.venv}"
PYTHON_BIN="${PYTHON:-python3}"

if ! command -v "$PYTHON_BIN" >/dev/null 2>&1; then
  echo "Python interpreter '$PYTHON_BIN' not found. Set PYTHON env var or install python3." >&2
  exit 1
fi

if [[ ! -d "$VENV_DIR" ]]; then
  echo "[bootstrap] Creating virtualenv at $VENV_DIR"
  "$PYTHON_BIN" -m venv "$VENV_DIR"
else
  echo "[bootstrap] Using existing virtualenv at $VENV_DIR"
fi

# shellcheck source=/dev/null
source "$VENV_DIR/bin/activate"

pip install --upgrade pip >/dev/null
pip install --upgrade platformio esptool >/dev/null

deactivate >/dev/null 2>&1 || true

echo "[bootstrap] PlatformIO + esptool installed in $VENV_DIR"
echo "[bootstrap] Activate it with 'source $VENV_DIR/bin/activate' or just run 'make build' (scripts auto-detect the venv)."
