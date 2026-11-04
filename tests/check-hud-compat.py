#!/usr/bin/env python3
"""Verify legacy HUD headers, use-site diagnostics and zero-cost source aliases."""
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[1]
sdcc = '--sdcc' in sys.argv
compiler = shlex.split(os.environ.get('SDCC', 'sdcc') if sdcc else os.environ.get('CC', 'cc'))
flags = ['-mz80', '--std-sdcc11', '--opt-code-size'] if sdcc else ['-std=c11', '-Wall', '-Wextra']
flags += ['-I' + str(root / 'include')]
if sdcc:
    flags += ['-I' + str(Path(os.environ['ZVB_SDK_PATH']) / 'include'),
              '-I' + str(Path(os.environ['ZOS_PATH']) / 'sdk/sdcc/include')]
else:
    flags += ['-I' + str(root / 'tests/hud/sdk')]

names = {
    'print_string': 'hud_tilemap_put_s',
    'print_string_layer': 'hud_tilemap_put_s_layer',
    'nprint_string': 'hud_tilemap_nput_s',
    'nprint_string_layer': 'hud_tilemap_nput_s_layer',
}
names.update({'hud_' + old: new for old, new in list(names.items())})
uses = '''
void a(gfx_context *c) { print_string(c, "A", 2, 3); }
void b(gfx_context *c) { print_string_layer(c, "A", 0, 2, 3); }
void d(gfx_context *c) { nprint_string(c, "A", 1, 2, 3); }
void e(gfx_context *c) { nprint_string_layer(c, "A", 1, 0, 2, 3); }
void (*address(void))(gfx_context *, const char *, uint8_t, uint8_t) { return print_string; }
'''
uses += re.sub(r'\b(print_string|print_string_layer|nprint_string|nprint_string_layer)\b',
               lambda m: 'hud_' + m[0], uses).replace('void a(', 'void ha(').replace(
               'void b(', 'void hb(').replace('void d(', 'void hd(').replace(
               'void e(', 'void he(').replace('(*address(', '(*haddress(')
direct = re.sub(r'\b(' + '|'.join(names) + r')\b', lambda m: names[m[0]], uses)

with tempfile.TemporaryDirectory(prefix='zgdk-hud-compat-') as directory:
    work = Path(directory)

    def compile_source(name, source, strict):
        path = work / (name + '.c')
        output = path.with_suffix('.rel' if sdcc else '.o')
        path.write_text(source)
        result = subprocess.run(compiler + flags +
            (['--Werror' if sdcc else '-Werror'] if strict else []) +
            ['-c', str(path), '-o', str(output)], capture_output=True, text=True)
        assert result.returncode == 0, result.stdout + result.stderr
        return output, result.stdout + result.stderr

    headers = ['zgdk/hud/tilemap.h', 'zgdk/hud.h', 'zgdk/hud/print.h', 'zgdk/hud/deprecated.h',
               'zgdk/utils/print.h', 'zgdk/utils.h']
    if sdcc:
        headers.append('zgdk.h')
    for i, header in enumerate(headers):
        include = '#include <' + header + '>\n'
        _, diagnostics = compile_source(f'quiet_{i}', include + direct, True)
        assert not diagnostics, diagnostics
        aliases, diagnostics = compile_source(f'alias_{i}', include + uses, False)
        for name in names:
            assert name + ' is deprecated; use ' + names[name] in diagnostics, diagnostics
        if sdcc:
            modern, diagnostics = compile_source(f'modern_{i}', include + direct, True)
            assert not diagnostics, diagnostics
            a, b = aliases.read_text(), modern.read_text()
            refs = lambda text: set(re.findall(r'^S (\S+) Ref', text, re.M))
            size = lambda text: int(re.search(r'^A _CODE size ([0-9A-Fa-f]+)', text, re.M)[1], 16)
            assert refs(a) == refs(b) == {'_' + name for name in names.values()}
            assert size(a) == size(b), 'Aliases must add no wrapper code'
print(('SDCC' if sdcc else 'host') + ': old headers and names work; headers stay quiet; aliases warn on use, including function pointers')
