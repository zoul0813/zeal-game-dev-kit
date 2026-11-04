#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d "${TMPDIR:-/tmp}/zgdk-clamp.XXXXXX")
trap 'rm -rf "$out"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/include" "$root/tests/utils/clamp.c" "$root/src/utils/"clamp_*.c -o "$out/clamp"
"$out/clamp"
