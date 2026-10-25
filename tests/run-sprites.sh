#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-sprites.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/tests/sprites/sdk" -I"$root/tests" -I"$root/include" \
    "$root/tests/sprites/sprites.c" "$root/src/sprites.c" -o "$binary"
"$binary"
