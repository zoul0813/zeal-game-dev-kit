#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -O1 -g -fsanitize=address,undefined \
    -I"$root/include" "$root/tests/anims.c" "$root"/src/anims/*.c -o "$out/anims"
"$out/anims"
echo 'anims: all passed (ASan/UBSan)'
