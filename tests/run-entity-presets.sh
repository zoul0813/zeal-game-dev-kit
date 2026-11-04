#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
: "${ZOS_PATH:?Set ZOS_PATH}" "${ZVB_SDK_PATH:?Set ZVB_SDK_PATH}" "${COREUTILS_PATH:?Set COREUTILS_PATH}"
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT
${CC:-cc} -std=gnu11 -O1 -g -w -fsanitize=address,undefined \
 '-D__sfr=static volatile unsigned char' '-D__at(x)=' -D__SDCC_VERSION_MAJOR=4 -D__SDCC_VERSION_MINOR=4 '-D__sdcccall(x)=' \
 -I"$root/include" -I"$ZOS_PATH/sdk/sdcc/include" -I"$ZVB_SDK_PATH/include" -I"$COREUTILS_PATH/libsrc/libcore" \
 "$root/tests/entity_presets.c" "$root"/src/entities/presets/*.c \
 "$root"/src/entities/*.c "$root"/src/anims/*.c "$root"/src/physics/body/*.c -o "$out/presets"
"$out/presets"
