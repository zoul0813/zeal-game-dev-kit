#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d "${TMPDIR:-/tmp}/zgdk-sprite-hud.XXXXXX")
trap 'rm -rf "$out"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/tests/hud/sdk" -I"$root/include" "$root/tests/hud/sprite.c" \
    "$root/src/hud/sprite/"*.c "$root/src/hud/tilemap/"*.c \
    "$root/src/ascii.c" "$root/src/utils/_strlen.c" -o "$out/hud"
"$out/hud"
