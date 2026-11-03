#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -O1 -g -fsanitize=address,undefined \
    -I"$root/tests/stream/sdk" -I"$root/include" \
    "$root/tests/entities.c" "$root"/src/entities/*.c -o "$out/entities"
"$out/entities"
echo 'entities: all passed (ASan/UBSan)'
