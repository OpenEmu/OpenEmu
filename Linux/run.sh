#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
if [[ -x .venv/bin/python ]]; then
    interpreter=.venv/bin/python
else
    interpreter="${PYTHON:-python3}"
fi
if ! "$interpreter" -c 'from PySide6 import QtWidgets, QtMultimedia' 2>/dev/null; then
    printf '%s\n' 'PySide6 is required. See Linux/README.md for setup instructions.' >&2
    exit 1
fi
exec "$interpreter" -m openemu_linux "$@"
