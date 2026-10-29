#!/bin/sh
# Needs Python 3 with Pillow and the Video Board SDK (ZVB_SDK_PATH, set by ZDE).
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
"${PYTHON:-python3}" -W ignore::DeprecationWarning "$root/tests/tileset/zts_chunks_test.py" "$@"
