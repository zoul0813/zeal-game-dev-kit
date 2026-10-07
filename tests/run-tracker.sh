#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
suite=${1:-volume}
extra=
if [ "$suite" = writes ]; then
    extra="$root/src/sound/tracker/write_exact.c $root/src/sound/tracker/pattern_save.c $root/src/sound/tracker/arrangement_save.c $root/src/sound/tracker/file_save.c $root/src/sound/tracker/copy_title_field.c"
fi
if [ "$suite" = rom ]; then
    extra="$root/src/sound/tracker/rom_load.c $root/src/sound/tracker/rom_read.c $root/src/sound/tracker/pattern_rom_load.c $root/src/sound/tracker/arrangement_rom_load.c"
fi
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-tracker.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -fsanitize=address,undefined -I"$root/tests/sdk" -I"$root/tests" -I"$root/include" \
    "$root/src/sound/tracker/main.c" "$root/src/sound/tracker/global.c" \
    "$root/src/sound/tracker/pattern_nav.c" "$root/src/sound/tracker/notes.c" \
    "$root/tests/tracker_hardware.c" "$root/tests/$suite.c" $extra -o "$binary"
"$binary"
