#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
suite=${1:-tile-offset}
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-header.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -fsanitize=address,undefined -I"$root/tests/sdk" -I"$root/include" "$root/tests/$suite.c" -o "$binary"
"$binary"
