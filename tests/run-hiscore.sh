#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d "${TMPDIR:-/tmp}/zgdk-hiscore.XXXXXX")
trap 'rm -rf "$out"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/tests/hiscore/sdk" -I"$root/include" \
    "$root/tests/hiscore/hiscore.c" "$root/src/hiscore/"*.c -o "$out/hiscore"
"$out/hiscore"
