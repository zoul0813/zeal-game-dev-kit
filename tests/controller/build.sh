#!/bin/sh
# Run inside the Zeal development environment, from the repository root.
set -eu
build=${CONTROLLER_BUILD_DIR:-build-controller}
mkdir -p "$build/reference" "$build/current"
for file in tests/controller/reference/*.c; do
    sdcc -mz80 --codeseg TEXT --opt-code-speed -Iinclude \
        -I"$ZOS_PATH/sdk/sdcc/include" -I"$ZVB_SDK_PATH/include" \
        -c "$file" -o "$build/reference/$(basename "$file" .c).rel"
done
sdldz80 -n -i -m -w -b _TEXT=0x2000 -b _DATA=0x8000 \
    -b _INITIALIZED=0x8100 -b _INITIALIZER=0x8200 \
    "$build/reference" "$build"/reference/*.rel
rm -f "$build/reference.lib"
sdar -rcsD "$build/reference.lib" "$build"/reference/*.rel
rm -f "$build"/current/*.rel
for file in src/input/controller/*.asm; do
    sdasz80 -o "$build/current/$(basename "$file" .asm).rel" "$file"
done
sdcc -mz80 --codeseg TEXT --opt-code-speed -Iinclude \
    -I"$ZOS_PATH/sdk/sdcc/include" -c tests/controller/calls.c -o "$build/current/calls.rel"
sdldz80 -n -i -m -w -b _TEXT=0x2000 -b _DATA=0x8000 \
    "$build/current" "$build"/current/*.rel
