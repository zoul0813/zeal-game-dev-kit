#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d "${TMPDIR:-/tmp}/zgdk-physics-z80.XXXXXX")
trap 'rm -rf "$out"' EXIT HUP INT TERM
: "${ZOS_PATH:?set ZOS_PATH}"
: "${ZVB_SDK_PATH:?set ZVB_SDK_PATH}"
# Compile production sources separately, as the library build does.
for source in "$root/src/physics/"*/*.c "$root/src/tilemap/flags/"*.c "$root/src/utils/clamp_i16.c"; do
    object=$(printf '%s' "${source#"$root/src/"}" | tr / _)
    "${SDCC:-sdcc}" -mz80 --opt-code-size -I"$root/include" \
        -I"$ZVB_SDK_PATH/include" -I"$ZOS_PATH/sdk/sdcc/include" \
        -c "$source" -o "$out/${object%.c}.rel"
done
for source in "$root/src/physics/tilemap/"*.asm; do
    "${AS:-sdasz80}" -o "$out/$(basename "$source" .asm).rel" "$source"
done
mkdir "$out/harnesses"
for test in probes platformer tileflags; do
    harness="$root/tests/physics/z80/probe_harness.c"
    if [ "$test" != probes ]; then harness="$root/tests/physics/z80/${test}_harness.c"; fi
    "${SDCC:-sdcc}" -mz80 --opt-code-size -I"$root/include" -I"$root/src" \
        -I"$ZVB_SDK_PATH/include" -I"$ZOS_PATH/sdk/sdcc/include" \
        --no-std-crt0 --code-loc 0x0100 --data-loc 0xC000 \
        -o "$out/harnesses/h_$test.ihx" "$harness" "$out/"*.rel
    "${MAKEBIN:-makebin}" -s 65536 "$out/harnesses/h_$test.ihx" "$out/harnesses/h_$test.bin"
done
"${PYTHON:-python3}" "$root/tests/physics/z80/probe_check.py" "$out/harnesses"
