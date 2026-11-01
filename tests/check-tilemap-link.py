#!/usr/bin/env python3
"""Check tilemap archive isolation after building with SDCC."""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
path = Path(sys.argv[1]) if len(sys.argv) > 1 else root / 'lib/zgdk.lib'
data = path.read_bytes()
assert data[:8] == b'!<arch>\n', 'Expected an ar library'
objects = []
offset = 8
while offset + 60 <= len(data):
    length = int(data[offset + 48:offset + 58])
    text = data[offset + 60:offset + 60 + length].decode('ascii', errors='replace')
    offset += 60 + length + (length % 2)
    if text.startswith('XL'):
        objects.append({
            'defs': set(re.findall(r'^S (\S+) Def', text, re.M)),
            'refs': set(re.findall(r'^S (\S+) Ref', text, re.M)),
        })

cases = {
    'register': {'register'},
    'get_xy': {'get_xy', 'register'},
    'get': {'get', 'get_xy', 'register'},
    'set_xy': {'set_xy', 'register'},
    'set': {'set', 'set_xy', 'register'},
    'fill': {'fill'},
    'fill_rect': {'fill_rect', 'fill'},
    'place': {'place', 'register'},
    'place_xy': {'place_xy'},
}
public = {'_tilemap_' + name for name in cases}
for symbol in public | {'__TILEMAP'}:
    assert sum(symbol in obj['defs'] for obj in objects) == 1, symbol

for name, expected in cases.items():
    wanted = {'_tilemap_' + name}
    defined = set()
    linked = []
    while True:
        obj = next((obj for obj in objects
                    if obj not in linked and obj['defs'] & (wanted - defined)), None)
        if obj is None:
            break
        linked.append(obj)
        defined |= obj['defs']
        wanted |= obj['refs']
    assert defined & public == {'_tilemap_' + entry for entry in expected}, name
    graphics = {symbol for symbol in wanted if symbol.startswith('_gfx_')}
    expected_graphics = ({'_gfx_tilemap_load'} if name.startswith('fill') else
                         {'_gfx_tilemap_place'} if name.startswith('place') else set())
    assert graphics == expected_graphics, (name, graphics)
print('tilemap: nine entry points isolated; one shared state; getters/setters need no graphics')
