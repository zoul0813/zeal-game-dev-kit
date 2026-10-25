#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-rect.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -g -Wall -Wextra -Werror -Wno-ignored-qualifiers \
    -fsanitize=address,undefined -I"$root/include" \
    "$root/tests/rect.c" "$root/src/rect/collide.c" -o "$binary"
"$binary"
