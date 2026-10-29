#!/bin/sh
# Run from any directory; no Zeal SDK or target toolchain required.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-file.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/tests/file/sdk" -I"$root/tests" -I"$root/include" \
    "$root/tests/file/read_safe.c" "$root/src/file/read_safe.c" -o "$binary"
"$binary"
