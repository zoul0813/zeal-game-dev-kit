#!/usr/bin/env python3
"""Link Z80 HUD consumers to check span layout and selective sprite capabilities."""
from pathlib import Path
import os
import re
import shlex
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
flags = shlex.split(os.environ.get('SDCC', 'sdcc')) + [
    '-mz80', '--opt-code-size', '--no-std-crt0', '--code-loc', '0x100', '--data-loc', '0xc000',
    '-I' + str(root / 'include'),
    '-I' + str(Path(os.environ['ZVB_SDK_PATH']) / 'include'),
    '-I' + str(Path(os.environ['ZOS_PATH']) / 'sdk/sdcc/include'),
]
common = '''#include <zgdk/hud.h>
typedef char z80_view_is_four_bytes[sizeof(hud_sprite_text_t) == 4 ? 1 : -1];
static gfx_sprite sprites[4];
static const char chars[4] = {'A','B','C','D'};
'''
cases = {
    'hide': 'void main(void) { hud_sprite_hide(sprites, 4); }',
    'counted': '''void main(void) {
    hud_sprite_text_t line = {sprites, 4, 12};
    hud_sprite_nput_s(&line, chars, 4, 10, 20);
}''',
    'centered': '''void main(void) {
    hud_sprite_text_t line = {sprites, 4, 12};
    hud_sprite_put_s_centered(&line, "AB", 160, 20);
}''',
    'mixed': '''static gfx_context ctx;
gfx_error gfx_tilemap_load(gfx_context *c, void *tiles, uint8_t n, uint8_t l, uint8_t x, uint8_t y) {
    (void)c; (void)tiles; (void)n; (void)l; (void)x; (void)y;
    return GFX_SUCCESS;
}
void main(void) {
    hud_sprite_text_t line = {sprites, 4, 12};
    hud_sprite_put_s(&line, "AB", 0, 0);
    hud_sprite_put_s_centered(&line, "AB", 160, 0);
    hud_sprite_nput_s(&line, chars, 4, 0, 0);
    hud_sprite_hide(sprites, 4);
    hud_tilemap_put_s(&ctx, "AB", 0, 0);
    hud_tilemap_put_s_layer(&ctx, "AB", 0, 0, 0);
    hud_tilemap_nput_s(&ctx, chars, 4, 0, 0);
    hud_tilemap_nput_s_layer(&ctx, chars, 4, 1, 0, 0);
}''',
}
expected = {
    'hide': {'hide'},
    'counted': {'nput_s'},
    'centered': {'put_s_centered', 'nput_s', 'text_length'},
    'mixed': {'hide', 'put_s', 'put_s_centered', 'nput_s', 'text_length'},
}
with tempfile.TemporaryDirectory(prefix='zgdk-sprite-hud-link-') as tmp:
    out = Path(tmp)
    for name, body in cases.items():
        source = out / (name + '.c'); source.write_text(common + body)
        subprocess.run(flags + ['-o', str(out / (name + '.ihx')), str(source),
                               '-L' + os.environ.get('ZGDK_LIB', str(root / 'lib')), '-l', 'zgdk'], check=True)
        mapping = (out / (name + '.map')).read_text()
        found = set(re.findall(r'\b_hud_sprite_(\w+)\b', mapping))
        assert found == expected[name], (name, found)
        if name == 'hide':
            assert '_ASCII_MAP' not in mapping, name
        else:
            assert '_ASCII_MAP' in mapping, name
        if name != 'mixed':
            assert not re.search(r'\b_hud_tilemap_\w+\b', mapping), name
        assert not re.search(r'\b_(?:sprites_register|gfx_sprite_render)\w*\b', mapping), name
print('sprite HUD target: four-byte view, shared ASCII mapping, mixed tilemap linkage, isolated hide/text, no arena')
