#!/bin/sh
# Build first with tests/build-sprites.sh (SDCC >= 4.2, ZVB_SDK_PATH).
# Run in a Python environment containing z80==1.2.0.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
"${PYTHON:-python3}" -B "$root/tests/sprites.py" "${1:-$root/build/sprite-tests}"
