#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d "${TMPDIR:-/tmp}/zgdk-physics-z80.XXXXXX")
trap 'rm -rf "$out"' EXIT HUP INT TERM
: "${ZOS_PATH:?set ZOS_PATH}"
: "${ZVB_SDK_PATH:?set ZVB_SDK_PATH}"
for variant in asm c; do
    def=""
    if [ "$variant" = c ]; then def=-DBODY_PROBE_C; fi
    "${SDCC:-sdcc}" -mz80 --opt-code-size -I"$root/include" -I"$root/src" \
        -I"$ZVB_SDK_PATH/include" -I"$ZOS_PATH/sdk/sdcc/include" \
        $def --no-std-crt0 --code-loc 0x0100 --data-loc 0xC000 \
        -o "$out/h_$variant.ihx" "$root/tests/physics/z80/probe_harness.c"
    "${MAKEBIN:-makebin}" -s 65536 "$out/h_$variant.ihx" "$out/h_$variant.bin"
done
"${PYTHON:-python3}" "$root/tests/physics/z80/probe_check.py" "$out"
