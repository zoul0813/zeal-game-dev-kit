#!/bin/sh
# Run the real input modules with simulated keyboard reads and pad reports.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
binary=$(mktemp "${TMPDIR:-/tmp}/zgdk-input.XXXXXX")
trap 'rm -f "$binary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
    -I"$root/tests/input/sdk" -I"$root/tests" -I"$root/include" \
    "$root/tests/input/input.c" "$root/src/input/input.c" \
    "$root/src/input/keyboard.c" "$root/src/input/keyboard_pressed.c" -o "$binary"
"$binary"
