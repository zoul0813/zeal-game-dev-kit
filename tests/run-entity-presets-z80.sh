#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
: "${ZOS_PATH:?Set ZOS_PATH}" "${ZVB_SDK_PATH:?Set ZVB_SDK_PATH}" "${COREUTILS_PATH:?Set COREUTILS_PATH}"
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT
for kind in sprite animated physics animated_physics pool; do
    sdcc -mz80 --opt-code-size -I"$root/include" -I"$ZOS_PATH/sdk/sdcc/include" -I"$ZVB_SDK_PATH/include" \
        --no-std-crt0 --code-loc 0x100 --data-loc 0xC000 \
        -o "$out/$kind.ihx" "$root/tests/entities/z80/$kind.c" \
        -l "$root/lib/zgdk.lib" -l "$COREUTILS_PATH/lib/core.lib"
done
${PYTHON:-python3} "$root/tests/entities/z80/preset_check.py" --suite "$out"
