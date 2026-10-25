#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-sound.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/tests/sound/sdk" -I"$root/tests/tracker/sdk" -I"$root/include" \
    "$root/tests/sound/sound.c" "$root/src/sound/sounds.c" -o "$binary"
"$binary"
