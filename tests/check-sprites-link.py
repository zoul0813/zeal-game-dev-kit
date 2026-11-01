#!/usr/bin/env python3
"""Check sprite archive isolation after building with SDCC."""
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

public = {
    '_sprites_register_arena', '_sprites_register', '_sprites_deregister',
    '_sprites_render', '_sprite_get_point', '_sprite_set_point', '_sprite_set_xy',
}
state = {'_sprites_arena', '_sprites_arena_size', '_sprites_registered', '_sprites_rendered'}
for symbol in public | state:
    assert sum(symbol in obj['defs'] for obj in objects) == 1, symbol
state_object = next(obj for obj in objects if '_sprites_arena' in obj['defs'])
assert state <= state_object['defs']
assert not state_object['defs'] & public, 'State must not pull in sprite functions'

for symbol in public:
    wanted = {symbol}
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
    assert defined & public == {symbol}, symbol
    assert bool(defined & state) == symbol.startswith('_sprites_'), symbol
    graphics = {name for name in wanted if name.startswith('_gfx_')}
    assert graphics == ({'_gfx_sprite_render_array'} if symbol == '_sprites_render' else set()), symbol
print('sprites: seven entry points isolated; one shared arena; point helpers need no state or graphics')
