#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d "${TMPDIR:-/tmp}/zgdk-inertial.XXXXXX")
trap 'rm -rf "$out"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/include" "$root/tests/physics/inertial.c" \
    "$root/src/physics/body/"*.c "$root/src/physics/inertial/update.c" "$root/src/utils/clamp_i16.c" -o "$out/inertial"
"$out/inertial"
