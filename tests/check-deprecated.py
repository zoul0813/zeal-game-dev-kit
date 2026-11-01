#!/usr/bin/env python3
"""Check automatic namespace aliases, SDCC diagnostics and link dependencies."""
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[1]
native = '--sdcc' in sys.argv
compiler = ['sdcc'] if native else shlex.split(os.environ.get('CC', 'cc'))
# Match the SDK's C11 dialect, including binary constants in ZVB headers.
flags = (['-mz80', '--std-sdcc11', '--codeseg', 'TEXT', '--opt-code-speed'] if native
         else ['-std=c11', '-Wall', '-Wextra'])
flags += ['-I' + str(root / 'include'), '-I' + str(
    Path(os.environ['ZOS_PATH']) / 'sdk/sdcc/include' if native else root / 'tests/input/sdk')]

modern = '#include <zgdk/input.h>\n'
quiet = modern + '''#include <zgdk/input/deprecated.h>
#ifndef input_get
#error input.h must include its deprecated aliases automatically
#endif
#ifndef controller_read
#error input.h must expose the supported controller_read alias
#endif
unsigned quiet(void) { return 0; }
'''
controller_only = '''#include <zgdk/input/controller.h>
#ifndef controller_read
#error controller.h must provide the supported controller_read alias
#endif
#ifdef input_get
#error controller.h must not expose unrelated input aliases
#endif
unsigned quiet(void) { return 0; }
'''
wrappers = '''uint16_t alias_input(void) { return input_get(); }
uint16_t alias_pad(void) { return controller_read(); }
uint16_t masked_buttons(uint16_t buttons, uint16_t mask) { return controller_pressed(buttons, mask); }
uint8_t valid_pad(uint16_t report) { return controller_pad_valid(report); }
'''

with tempfile.TemporaryDirectory(prefix='zgdk-deprecated-') as directory:
    work = Path(directory)

    def compile_source(name, source, strict=False):
        path = work / (name + '.c')
        output = path.with_suffix('.rel' if native else '.o')
        path.write_text(source)
        result = subprocess.run(compiler + flags +
            (['--Werror'] if native and strict else ['-Werror'] if strict else []) +
            ['-c', str(path), '-o', str(output)], capture_output=True, text=True)
        assert result.returncode == 0, result.stdout + result.stderr
        return output, result.stdout + result.stderr

    _, diagnostics = compile_source('quiet', quiet, strict=True)
    assert not diagnostics, diagnostics
    _, diagnostics = compile_source('controller_only', controller_only, strict=True)
    assert not diagnostics, diagnostics
    # The input deprecated header also supports being included first.
    _, diagnostics = compile_source('input_first',
        '#include <zgdk/input/deprecated.h>\n' + quiet, strict=True)
    assert not diagnostics, diagnostics
    _, diagnostics = compile_source('controller_call', '#include <zgdk/input/controller.h>\n' +
        'uint16_t alias_pad(void) { return controller_read(); }\n', strict=True)
    assert not diagnostics, diagnostics
    if native:
        # Verify the umbrella header used by existing games with real SDK headers.
        flags += ['-I' + str(Path(os.environ['ZVB_SDK_PATH']) / 'include')]
        _, diagnostics = compile_source('umbrella', '#include <zgdk.h>\n' + wrappers)
        assert 'input_get() is deprecated' in diagnostics, diagnostics
        assert 'controller_read() is deprecated' not in diagnostics, diagnostics
        _, diagnostics = compile_source('umbrella_supported', '#include <zgdk.h>\n' +
            'uint16_t alias_pad(void) { return controller_read(); }\n', strict=True)
        assert not diagnostics, diagnostics
    aliases, diagnostics = compile_source('aliases', modern + wrappers)
    assert 'input_get() is deprecated; use input_read()' in diagnostics, diagnostics
    assert 'controller_read() is deprecated' not in diagnostics, diagnostics

    if native:
        direct, diagnostics = compile_source('direct', modern + wrappers.replace(
            'input_get()', 'input_read()').replace('controller_read()', 'controller_read_pad(SNES_PORT1)').replace(
            'controller_pressed(buttons, mask)', '((buttons) & (mask))').replace(
            'controller_pad_valid(report)', '(((report) & 0xF000) == 0)'), strict=True)
        assert not diagnostics, diagnostics
        refs = lambda text: set(re.findall(r'^S (\S+) Ref', text, re.M))
        size = lambda text: int(re.search(r'^A _TEXT size ([0-9A-Fa-f]+)', text, re.M)[1], 16)
        a, b = aliases.read_text(), direct.read_text()
        assert refs(a) == refs(b) == {'_input_read', '_controller_read_pad'}
        assert size(a) == size(b), 'Aliases must not add wrapper code'
    else:
        driver = work / 'driver.c'
        driver.write_text('''#include <assert.h>
#include <zgdk/input/controller.h>
static unsigned inputs, pads;
uint16_t input_read(void) { ++inputs; return 0x1234; }
uint16_t controller_read_pad(uint8_t port) { assert(port == 0); ++pads; return 0xabcd; }
uint16_t alias_input(void);
uint16_t alias_pad(void);
uint16_t masked_buttons(uint16_t buttons, uint16_t mask);
uint8_t valid_pad(uint16_t report);
int main(void) {
    assert(alias_input() == 0x1234 && inputs == 1 && pads == 0);
    assert(alias_pad() == 0xabcd && inputs == 1 && pads == 1);
    assert(masked_buttons(0, BUTTON_A | BUTTON_Y) == 0);
    assert(masked_buttons(BUTTON_B, BUTTON_A | BUTTON_Y) == 0);
    assert(masked_buttons(BUTTON_A, BUTTON_A | BUTTON_Y) == BUTTON_A);
    assert(masked_buttons(BUTTON_Y, BUTTON_A | BUTTON_Y) == BUTTON_Y);
    assert(masked_buttons(BUTTON_A | BUTTON_Y, BUTTON_A | BUTTON_Y) == (BUTTON_A | BUTTON_Y));
    assert(masked_buttons(0xffff, 0) == 0);
    uint16_t buttons[] = { BUTTON_A, 0 }, masks[] = { BUTTON_A | BUTTON_Y, 0 };
    uint16_t *b = buttons, *m = masks;
    assert(controller_pressed(*b++, *m++) == BUTTON_A);
    assert(b == buttons + 1 && m == masks + 1);
    assert(controller_pressed(BUTTON_A | BUTTON_Y, BUTTON_Y) == BUTTON_Y);
    /* Cover every 16-bit signature/button combination, including 0xFFFF. */
    for (unsigned report = 0; report <= 0xffff; ++report) {
        assert(valid_pad(report) == (report < 0x1000));
    }
    b = buttons;
    assert(controller_pad_valid(*b++));
    assert(b == buttons + 1);
    assert(!controller_pad_valid(MOUSE_ID | MOUSE_LEFT));
    assert(inputs == 1 && pads == 1);
    return 0;
}
''')
        binary = work / 'aliases'
        subprocess.run(compiler + flags + ['-Werror', str(driver), str(aliases), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
print(('SDCC' if native else 'host') + ': input_get warns on use; supported controller_read stays quiet; aliases add no wrapper code')
