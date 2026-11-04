#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-rooms.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
for suite in rooms minimap; do
    if [ "$suite" = rooms ]; then
        set -- "$root/src/rooms/rooms.c" "$root/src/rooms/transitions.c" "$root/src/rooms/slide.c" \
            "$root/src/tilemap/register.c" "$root/src/tilemap/get.c" \
            "$root/src/tilemap/get_xy.c" "$root/src/tilemap/set_xy.c"
    else
        set -- "$root/src/minimap/state.c" "$root/src/minimap/render.c" "$root/src/minimap/minimap.c"
    fi
    "${CC:-cc}" -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
        -I"$root/tests/rooms/sdk" -I"$root/tests/rooms" -I"$root/include" \
        "$root/tests/rooms/$suite.c" "$root/tests/rooms/support.c" "$@" -o "$binary"
    "$binary"
done
