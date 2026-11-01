#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build=${CONTROLLER_BUILD_DIR:-"$root/build-controller"}
emulator=${ZEAL_EMULATOR_PATH:-"$root/../Zeal-NativeEmulator"}
if [ ! -f "$emulator/hw/z80.c" ]; then
    echo "Set ZEAL_EMULATOR_PATH to a Zeal-NativeEmulator checkout." >&2
    exit 1
fi
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-controller.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -g -fsanitize=address,undefined \
    -I"$emulator/include" -Wno-deprecated-declarations \
    "$root/tests/controller/controller.c" "$emulator/hw/z80.c" \
    "$emulator/hw/mmu.c" "$emulator/utils/vtimer.c" -o "$binary"
"$binary" "$build/reference.ihx" "$build/reference.map" "$build/current.ihx" "$build/current.map"
