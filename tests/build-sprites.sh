#!/bin/sh
# SDCC >= 4.2, with the ZVB SDK headers used by games.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=${1:-"$root/build/sprite-tests"}
mkdir -p "$out"
set --
for source in "$root"/src/sprites/*.asm; do
    sdasz80 -l -s -o "$out/$(basename "$source" .asm).rel" "$source"
    set -- "$@" "$out/$(basename "$source" .asm).rel"
done
rm -f "$out/sprites.lib"
sdar -rc "$out/sprites.lib" "$@"
sdcc -mz80 --codeseg TEXT --opt-code-speed -I"$root/include" -I"$ZVB_SDK_PATH/include" \
    -c "$root/tests/sprites_api.c" -o "$out/api.rel"
# The SDK maps VRAM into page zero, so execute game code in page one.
sdcc -mz80 --no-std-crt0 -Wl-w --code-loc 0x4000 --data-loc 0x9000 -Wl-b_TEXT=0x6000 \
    "$out/api.rel" "$out/sprites.lib" -o "$out/api.ihx"
# Deliberately place the 24-byte descriptor buffer across a 16KB boundary.
sdcc -mz80 --no-std-crt0 -Wl-w --code-loc 0x4000 --data-loc 0xbfdf -Wl-b_TEXT=0x6000 \
    "$out/api.rel" "$out/sprites.lib" -o "$out/boundary.ihx"
