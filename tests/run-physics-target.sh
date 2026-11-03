#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d "${TMPDIR:-/tmp}/zgdk-physics-link.XXXXXX")
trap 'rm -rf "$out"' EXIT HUP INT TERM
: "${ZOS_PATH:?set ZOS_PATH}"
: "${ZVB_SDK_PATH:?set ZVB_SDK_PATH}"
for test in free collision platformer; do
    "${SDCC:-sdcc}" -mz80 --opt-code-size -I"$root/include" \
        -I"$ZVB_SDK_PATH/include" -I"$ZOS_PATH/sdk/sdcc/include" \
        --no-std-crt0 --code-loc 0x0100 --data-loc 0xc000 \
        -o "$out/$test.ihx" "$root/tests/physics/target/$test.c" \
        -L"${ZGDK_LIB:-$root/lib}" -l zgdk
done
for helper in EDGES ROW COLUMN; do
    "${SDCC:-sdcc}" -mz80 --opt-code-size -I"$root/include" -I"$root/src" \
        -I"$ZVB_SDK_PATH/include" -I"$ZOS_PATH/sdk/sdcc/include" \
        -DTEST_$helper --no-std-crt0 --code-loc 0x0100 --data-loc 0xc000 \
        -o "$out/helper_$helper.ihx" "$root/tests/physics/target/helper.c" \
        -L"${ZGDK_LIB:-$root/lib}" -l zgdk
done
"${PYTHON:-python3}" "$root/tests/physics/target/check.py" "$out"
