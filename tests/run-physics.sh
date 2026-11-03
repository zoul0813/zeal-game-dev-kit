#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d "${TMPDIR:-/tmp}/zgdk-physics.XXXXXX")
trap 'rm -rf "$out"' EXIT HUP INT TERM
# Free bodies intentionally link no collision/tilemap/cache/controller symbols.
"${CC:-cc}" -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/include" "$root/tests/physics/free.c" "$root/src/physics/body/"*.c -o "$out/free"
"$out/free"
# Collision behavior runs on Z80 now that the probes have a single assembly implementation.
sh "$root/tests/run-physics-z80.sh"
