#!/usr/bin/env python3
"""Check the native archive after zde cmake: ROM playback must not link file I/O."""
from pathlib import Path
import re
import sys

path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1] / "lib/zgdk.lib"
data = path.read_bytes()
assert data[:8] == b"!<arch>\n", "Expected an ar library"
objects = []
offset = 8
while offset + 60 <= len(data):
    length = int(data[offset + 48:offset + 58])
    text = data[offset + 60:offset + 60 + length].decode("ascii", errors="replace")
    offset += 60 + length + (length % 2)
    if not text.startswith("XL"):
        continue  # Archive index and long filename table.
    objects.append({
        "defs": set(re.findall(r"^S (\S+) Def", text, re.M)),
        "refs": set(re.findall(r"^S (\S+) Ref", text, re.M)),
        "code": sum(int(size, 16) for size in re.findall(r"^A _TEXT size ([0-9A-Fa-f]+)", text, re.M)),
    })

for helper in ("DIV", "WAVE", "SELECT", "OFF", "VOL", "RESET"):
    symbol = "_SOUND_" + helper
    assert sum(symbol in obj["defs"] for obj in objects) == 1, symbol + " needs one shared definition"

wanted = {"_zmt_tick", "_zmt_reset", "_zmt_rom_load", "_zmt_track_reset"}
defined = set()
linked = []
while True:
    obj = next((obj for obj in objects if obj not in linked and obj["defs"] & (wanted - defined)), None)
    if obj is None:
        break
    linked.append(obj)
    defined |= obj["defs"]
    wanted |= obj["refs"]

file_io = {"_read", "_write", "_open", "_close", "_palloc", "_pfree", "_tracker_read_exact", "_tracker_write_exact"}
assert not wanted & file_io, "ROM playback unexpectedly references file I/O: " + str(wanted & file_io)
for symbol in ("_NOTES", "_zmt_ticks"):
    obj = next(obj for obj in linked if symbol in obj["defs"])
    assert obj["code"] == 0, symbol + " data module contains unused helper code"
print("native tracker link checks passed: shared helpers, no ROM file I/O, no code in data modules")
