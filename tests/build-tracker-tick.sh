#!/bin/sh
# SDCC >= 4.2; ZOS_PATH, ZVB_SDK_PATH and COREUTILS_PATH as for CMake.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=${1:-"$root/build/tick-tests"}
source=${2:-"$root/src/sound/tracker/main.c"}
mkdir -p "$out"
for unit in main global pattern_nav notes; do
    file="$root/src/sound/tracker/$unit.c"
    if [ "$unit" = main ]; then file=$source; fi
    sdcc -mz80 --codeseg TEXT --opt-code-speed \
        -I"$ZOS_PATH/sdk/sdcc/include" -I"$ZVB_SDK_PATH/include" \
        -I"$COREUTILS_PATH/libsrc/libcore" -I"$root/include" -I"$root/src/sound/tracker" \
        -c "$file" -o "$out/$unit.rel"
done
sdcc -mz80 --codeseg TEXT --opt-code-speed -I"$ZVB_SDK_PATH/include" -I"$ZOS_PATH/sdk/sdcc/include" -I"$root/include" \
    -c "$root/tests/tracker_tick_api.c" -o "$out/api.rel"
sdcc -mz80 --no-std-crt0 -Wl-w --code-loc 0x4000 --data-loc 0x9000 -Wl-b_TEXT=0x5000 \
    "$out/api.rel" "$out/main.rel" "$out/global.rel" "$out/pattern_nav.rel" "$out/notes.rel" \
    -o "$out/api.ihx"
