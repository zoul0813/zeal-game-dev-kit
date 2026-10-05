#!/bin/sh
# Run from any directory; no Zeal SDK or target toolchain required.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-tiles.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -I"$root/tests" -I"$root/include" \
    "$root/src/tiles.c" "$root/tests/tiles.c" -o "$binary"
"$binary"
