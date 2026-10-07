#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
suite=${1:-volume}
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-tracker.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -fsanitize=address,undefined -I"$root/tests/sdk" -I"$root/tests" -I"$root/include" \
    "$root/src/sound/tracker/main.c" "$root/src/sound/tracker/global.c" \
    "$root/src/sound/tracker/pattern_nav.c" "$root/src/sound/tracker/notes.c" \
    "$root/tests/tracker_hardware.c" "$root/tests/$suite.c" -o "$binary"
"$binary"
