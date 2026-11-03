#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d "${TMPDIR:-/tmp}/zgdk-physics.XXXXXX")
trap 'rm -rf "$out"' EXIT HUP INT TERM
# Free bodies intentionally link no collision/tilemap/cache/controller symbols.
"${CC:-cc}" -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/include" "$root/tests/physics/free.c" "$root/src/physics/body.c" \
    "$root/src/physics/overlap.c" -o "$out/free"
"$out/free"
for test in platformer tileflags; do
    "${CC:-cc}" -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
        -I"$root/tests/stream/sdk" -I"$root/include" \
        "$root/tests/physics/$test.c" "$root/src/physics/"*.c "$root/src/physics/platformer/"*.c \
        -o "$out/$test"
    "$out/$test"
done
