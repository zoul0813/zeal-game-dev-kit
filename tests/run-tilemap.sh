#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-tilemap.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/tests/stream/sdk" -I"$root/include" \
    "$root/tests/tilemap/load.c" "$root/src/tilemap/load.c" \
    "$root/src/tilemap/register.c" "$root/src/tilemap/get.c" \
    "$root/src/tilemap/get_xy.c" -o "$binary"
"$binary"
