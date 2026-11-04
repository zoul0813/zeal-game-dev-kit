#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d "${TMPDIR:-/tmp}/zgdk-hud-z80.XXXXXX")
trap 'rm -rf "$out"' EXIT HUP INT TERM
"${SDCC:-sdcc}" -mz80 --opt-code-size --no-std-crt0 --code-loc 0x100 --data-loc 0xc000 \
    -I"$root/include" -I"$ZOS_PATH/sdk/sdcc/include" -I"$ZVB_SDK_PATH/include" \
    -o "$out/hud.ihx" "$root/tests/hud/sprite_target.c" -L"${ZGDK_LIB:-$root/lib}" -lzgdk
"${PYTHON:-python3}" "$root/tests/check-hud-sprite-z80.py" "$out/hud.ihx"
