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
flags = (['-mz80', '--std-c11', '--codeseg', 'TEXT', '--opt-code-speed'] if native
         else ['-std=c11', '-Wall', '-Wextra'])
flags += ['-I' + str(root / 'include'), '-I' + str(
    Path(os.environ['ZOS_PATH']) / 'sdk/sdcc/include' if native else root / 'tests/input/sdk')]

modern = '#include <zgdk/input.h>\n'
quiet = modern + '''#include <zgdk/input/deprecated.h>
#include <zgdk/input/controller/deprecated.h>
#ifndef input_get
#error input.h must include its deprecated aliases automatically
#endif
#ifndef controller_read
#error input.h must expose the controller namespace aliases
#endif
unsigned quiet(void) { return 0; }
'''
controller_only = '''#include <zgdk/input/controller.h>
#include <zgdk/input/controller/deprecated.h>
#ifndef controller_read
#error controller.h must include its deprecated aliases automatically
#endif
#ifdef input_get
#error controller.h must not expose unrelated input aliases
#endif
unsigned quiet(void) { return 0; }
'''
wrappers = '''uint16_t alias_input(void) { return input_get(); }
uint16_t alias_pad(void) { return controller_read(); }
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
    # Standalone nested headers also handle both include orders without warnings.
    for name, header in [('input_first', 'zgdk/input/deprecated.h'),
                         ('controller_first', 'zgdk/input/controller/deprecated.h')]:
        _, diagnostics = compile_source(name, '#include <' + header + '>\n' +
            (quiet if name == 'input_first' else controller_only), strict=True)
        assert not diagnostics, diagnostics
    _, diagnostics = compile_source('controller_call', '#include <zgdk/input/controller.h>\n' +
        'uint16_t alias_pad(void) { return controller_read(); }\n')
    assert 'controller_read() is deprecated' in diagnostics, diagnostics
    assert 'input_get() is deprecated' not in diagnostics, diagnostics
    if native:
        # Verify the umbrella header used by existing games with real SDK headers.
        flags += ['-I' + str(Path(os.environ['ZVB_SDK_PATH']) / 'include')]
        _, diagnostics = compile_source('umbrella', '#include <zgdk.h>\n' + wrappers)
        assert 'input_get() is deprecated' in diagnostics, diagnostics
        assert 'controller_read() is deprecated' in diagnostics, diagnostics
    aliases, diagnostics = compile_source('aliases', modern + wrappers)
    for message in ('input_get() is deprecated; use input_read()',
                    'controller_read() is deprecated; use controller_read_pad(SNES_PORT1)'):
        assert message in diagnostics, diagnostics

    if native:
        direct, diagnostics = compile_source('direct', modern + wrappers.replace(
            'input_get()', 'input_read()').replace('controller_read()', 'controller_read_pad(SNES_PORT1)'), strict=True)
        assert not diagnostics, diagnostics
        refs = lambda text: set(re.findall(r'^S (\S+) Ref', text, re.M))
        size = lambda text: int(re.search(r'^A _TEXT size ([0-9A-Fa-f]+)', text, re.M)[1], 16)
        a, b = aliases.read_text(), direct.read_text()
        assert refs(a) == refs(b) == {'_input_read', '_controller_read_pad'}
        assert size(a) == size(b), 'Aliases must not add wrapper code'
    else:
        driver = work / 'driver.c'
        driver.write_text('''#include <assert.h>
#include <stdint.h>
static unsigned inputs, pads;
uint16_t input_read(void) { ++inputs; return 0x1234; }
uint16_t controller_read_pad(uint8_t port) { assert(port == 0); ++pads; return 0xabcd; }
uint16_t alias_input(void);
uint16_t alias_pad(void);
int main(void) {
    assert(alias_input() == 0x1234 && inputs == 1 && pads == 0);
    assert(alias_pad() == 0xabcd && inputs == 1 && pads == 1);
    return 0;
}
''')
        binary = work / 'aliases'
        subprocess.run(compiler + flags + ['-Werror', str(driver), str(aliases), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
print(('SDCC' if native else 'host') + ': automatic namespace aliases warn on use, quiet on include, canonical APIs stay clean')
