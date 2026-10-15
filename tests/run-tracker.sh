#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-tracker.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -g -fsanitize=address,undefined \
    -Wno-unused-value -I"$root/tests/tracker/sdk" -I"$root/tests" \
    -I"$root/include" -I"$root/src/sound/tracker" \
    "$root/tests/tracker/tracker.c" \
    "$root/src/sound/tracker/main.c" \
    "$root/src/sound/tracker/global.c" \
    "$root/src/sound/tracker/sound_control.c" \
    "$root/src/sound/tracker/notes.c" \
    "$root/src/sound/tracker/pattern_nav.c" -o "$binary"
"$binary"
