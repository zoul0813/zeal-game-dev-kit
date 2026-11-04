#!/usr/bin/env python3
"""Check canonical HUD text entry points and absence of legacy wrapper code."""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
data = (Path(sys.argv[1]) if len(sys.argv) > 1 else root / 'lib/zgdk.lib').read_bytes()
assert data[:8] == b'!<arch>\n'
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
names = ('print_string', 'print_string_layer', 'nprint_string', 'nprint_string_layer')
legacy = {'_' + name for name in names} | {'_hud_' + name for name in names}
modern = {'_hud_tilemap_' + name for name in ('put_s', 'put_s_layer', 'nput_s', 'nput_s_layer')}
for symbol in modern:
    members = [obj for obj in objects if symbol in obj['defs']]
    assert len(members) == 1, symbol
    assert members[0]['defs'] & (legacy | modern) == {symbol}, symbol
for obj in objects:
    assert not obj['defs'] & legacy, ('Legacy names must be source aliases only', obj['defs'] & legacy)
    assert not obj['refs'] & legacy, ('SDK code must use canonical HUD names', obj['refs'] & legacy)
counted = next(obj for obj in objects if '_hud_tilemap_nput_s_layer' in obj['defs'])
assert counted['refs'] == {'_ascii_to', '_gfx_tilemap_load'}, counted['refs']
print('HUD: four canonical entry points isolated; legacy names add no symbols or wrapper code')
