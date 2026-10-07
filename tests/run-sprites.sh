#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-sprites.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -fsanitize=address,undefined -I"$root/tests/sdk" -I"$root/tests" -I"$root/include" \
    "$root/src/sprites.c" "$root/tests/sprites.c" -o "$binary"
"$binary"
