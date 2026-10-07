#!/bin/sh
# Requires SDCC >= 4.2 (default Z80 call(1)) and ZOS_PATH. Run from any directory.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=${1:-"$root/build/controller-tests"}
mkdir -p "$out"
zos_include="$ZOS_PATH/sdk/sdcc/include"
if [ ! -d "$zos_include" ]; then
    zos_include="$ZOS_PATH/kernel_headers/sdcc/include"
fi
set --
for source in "$root"/src/input/controller/*.asm; do
    sdasz80 -l -s -o "$out/$(basename "$source" .asm).rel" "$source"
    set -- "$@" "$out/$(basename "$source" .asm).rel"
done
rm -f "$out/controller.lib"
sdar -rc "$out/controller.lib" "$@"
sdcc -mz80 --codeseg TEXT --opt-code-speed -I"$root/include" -I"$zos_include" \
    -c "$root/tests/controller_api.c" -o "$out/api.rel"
sdcc -mz80 --no-std-crt0 -Wl-w --code-loc 0x1000 --data-loc 0x9000 \
    "$out/api.rel" "$out/controller.lib" -o "$out/api.ihx"
for mode in pad mouse; do
    define=
    if [ "$mode" = mouse ]; then define=-DTEST_MOUSE; fi
    sdcc -mz80 --codeseg TEXT --opt-code-speed -I"$root/include" -I"$zos_include" \
        $define -c "$root/tests/controller_link.c" -o "$out/$mode.rel"
    sdcc -mz80 --no-std-crt0 -Wl-w --code-loc 0x1000 --data-loc 0x9000 \
        "$out/$mode.rel" "$out/controller.lib" -o "$out/$mode.ihx"
done
