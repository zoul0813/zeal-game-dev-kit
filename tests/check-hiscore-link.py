#!/usr/bin/env python3
"""Verify hiscore archive members and the HUD query dependency boundary."""
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

public = {'_hiscore_' + name for name in ('init', 'show', 'hide', 'add', 'save', 'load', 'get', 'find')}
state = {'_hiscore_table', '_hiscore_config', '_hiscore_last_initials'}
for symbol in public | state:
    members = [obj for obj in objects if symbol in obj['defs']]
    assert len(members) == 1, symbol
    assert members[0]['defs'] & public == ({symbol} if symbol in public else set()), symbol
assert any(state <= obj['defs'] for obj in objects), 'Expected one shared state member'

for symbol in ('_hiscore_get', '_hiscore_find'):
    wanted, defined, linked = {symbol}, set(), []
    while True:
        obj = next((obj for obj in objects
                    if obj not in linked and obj['defs'] & (wanted - defined)), None)
        if obj is None:
            break
        linked.append(obj)
        defined |= obj['defs']
        wanted |= obj['refs']
    assert defined & public == {symbol}, (symbol, defined & public)
    assert not (wanted - defined), (symbol, 'unexpected external dependencies', wanted - defined)
    assert len(linked) == 2, (symbol, 'expected only query and shared state')
print('hiscores: eight independent entry points; HUD queries link only shared state, without UI/input/file I/O')
