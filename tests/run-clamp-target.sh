#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$(mktemp -d "${TMPDIR:-/tmp}/zgdk-clamp-link.XXXXXX")
trap 'rm -rf "$out"' EXIT HUP INT TERM
for kind in i16 u16 i8 u8; do
    case "$kind" in
        i16) type=int16_t ;; u16) type=uint16_t ;;
        i8) type=int8_t ;; u8) type=uint8_t ;;
    esac
    cat > "$out/$kind.c" <<SRC
#include <zgdk/utils/math.h>
static volatile $type value, result;
void main(void) { result = clamp_$kind(value, 1, 100); }
SRC
    "${SDCC:-sdcc}" -mz80 --opt-code-size -I"$root/include" \
        --no-std-crt0 --code-loc 0x0100 --data-loc 0xc000 \
        -o "$out/$kind.ihx" "$out/$kind.c" -L"${ZGDK_LIB:-$root/lib}" -l zgdk
done
"${PYTHON:-python3}" - "$out" <<'PY'
from pathlib import Path
import re, sys
for kind in ('i16', 'u16', 'i8', 'u8'):
    text = (Path(sys.argv[1]) / (kind + '.map')).read_text()
    found = set(re.findall(r'\b_clamp_(?:i16|u16|i8|u8)\b', text))
    assert found == {'_clamp_' + kind}, (kind, found)
print('clamp variants: each target links only its requested function')
PY
