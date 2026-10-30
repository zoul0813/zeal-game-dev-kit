#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d "${TMPDIR:-/tmp}/zgdk-stream.XXXXXX")
trap 'rm -rf "$out"' EXIT HUP INT TERM
compile_test() {
    "${CC:-cc}" -std=c11 -g -Wall -Wextra -Werror -fsanitize=address,undefined \
        -I"$root/tests/stream/sdk" -I"$root/include" "$@"
}
compile_test "$root/tests/stream/stream.c" "$root/src/camera/init_view.c" "$root/src/camera/set.c" \
    "$root/src/camera/follow_target.c" "$root/src/camera/scroll_layer.c" \
    "$root/src/camera/rebase.c" "$root/src/tilemap/stream.c" \
    "$root/src/tilemap/stream_reader.c" "$root/src/tilemap/stream_edit.c" \
    "$root/src/tilemap/stream_ram.c" "$root/src/tilemap/stream_rebase.c" \
    "$root/src/tilemap/stream_ring.c" \
    "$root/src/tilemap/stream_buffer.c" -o "$out/camera-stream"
"$out/camera-stream"
compile_test "$root/tests/stream/buffer.c" "$root/src/tilemap/stream.c" \
    "$root/src/tilemap/stream_reader.c" "$root/src/tilemap/stream_edit.c" \
    "$root/src/tilemap/stream_ram.c" "$root/src/tilemap/stream_rebase.c" \
    "$root/src/tilemap/stream_ring.c" \
    "$root/src/tilemap/stream_buffer.c" -o "$out/stream-only"
"$out/stream-only"
compile_test -I"$root/src/tilemap" "$root/tests/stream/ring.c" "$root/src/tilemap/stream_ring.c" -o "$out/ring"
"$out/ring"
compile_test "$root/tests/stream/target/reader_only.c" "$root/src/tilemap/stream.c" \
    "$root/src/tilemap/stream_reader.c" "$root/src/tilemap/stream_rebase.c" \
    "$root/src/tilemap/stream_ring.c" -o "$out/reader-only"
"$out/reader-only"
