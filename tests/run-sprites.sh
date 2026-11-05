#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-sprites.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/tests/sprites/sdk" -I"$root/tests" -I"$root/include" \
    "$root/tests/sprites/sprites.c" \
    "$root/src/sprites/state.c" "$root/src/sprites/register_arena.c" \
    "$root/src/sprites/register.c" "$root/src/sprites/deregister.c" \
    "$root/src/sprites/render.c" "$root/src/sprites/clear.c" \
    "$root/src/sprites/get_point.c" "$root/src/sprites/set_point.c" \
    "$root/src/sprites/set_xy.c" -o "$binary"
"$binary"

"${CC:-cc}" -std=c11 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/tests/sprites/sdk" -I"$root/tests" -I"$root/include" \
    "$root/tests/sprites/dma.c" "$root/src/sprites/dma.c" -o "$binary"
"$binary"
"$binary" absent
