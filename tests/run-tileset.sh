#!/bin/sh
# Run from any directory; no Zeal SDK or target toolchain required.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-tileset.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/tests/tileset/sdk" -I"$root/tests" -I"$root/include" \
    "$root/tests/tileset/tileset.c" "$root/src/tileset/load_chunks.c" -o "$binary"
"$binary"
