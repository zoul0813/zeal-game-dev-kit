#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-inline.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -O0 -g -Wall -Wextra -Werror \
    -fsanitize=address,undefined -I"$root/include" \
    "$root/tests/inline/main.c" "$root/tests/inline/peer.c" -o "$binary"
"$binary"
