#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-sfx.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/tests/sound/sdk" -I"$root/tests/tracker/sdk" -I"$root/include" \
    "$root/tests/sound/sfx.c" "$root/src/sound/sfx.c" "$root/src/sound/hardware.c" -o "$binary"
"$binary"
