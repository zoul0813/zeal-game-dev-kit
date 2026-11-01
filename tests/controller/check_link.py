#!/usr/bin/env python3
"""Real SDCC archive extraction and per-capability footprint checks."""
import os
from pathlib import Path
import re
import subprocess
import sys

build = Path(sys.argv[1] if len(sys.argv)>1 else 'build-controller') / 'links'
build.mkdir(parents=True, exist_ok=True)
library=Path(sys.argv[2] if len(sys.argv)>2 else 'lib/zgdk.lib')
base = {'_controller_init'}
scalar = {'_ctrl_latch','_ctrl_clock','_ctrl_wait','_ctrl_read16'}
pad = {'_controller_read_pad'} | scalar
mouse = {'_controller_read_mouse','_ctrl_mouse_byte','_ctrl_mouse_gap','_ctrl_mouse_start'} | scalar
speed = {'_controller_get_mouse_sensitivity','_ctrl_sensitivity_header','_ctrl_wait','_ctrl_read16'}
setter = speed | {'_controller_set_mouse_sensitivity'}
cases = {
    'init': ('',set()),
    'pad1': ('controller_read_pad(SNES_PORT1);',pad),
    'pad2': ('controller_read_pad(SNES_PORT2);',pad),
    'pads': ('ControllerPads p; controller_read_pads(&p);', {'_controller_read_pads','_ctrl_latch','_ctrl_clock','_ctrl_wait'}),
    'mouse1': ('ControllerMouse m; controller_read_mouse(SNES_PORT1,&m);',mouse),
    'mouse2': ('ControllerMouse m; controller_read_mouse(SNES_PORT2,&m);',mouse),
    'is_pad': ('controller_is_pad(SNES_PORT1);',pad|{'_controller_is_pad'}),
    'is_mouse': ('controller_is_mouse(SNES_PORT2);',mouse|{'_controller_is_mouse'}),
    'get_speed': ('controller_get_mouse_sensitivity(SNES_PORT1);',speed),
    'set_speed': ('controller_set_mouse_sensitivity(SNES_PORT2,MOUSE_HIGH);',setter),
    'mixed': ('ControllerMouse m; controller_read_pad(SNES_PORT1); controller_read_mouse(SNES_PORT2,&m);',pad|mouse),
    'input': ('input_init(1); input_read(); input_flush();',pad),
}
# OS endpoints are irrelevant to archive extraction; the actual input and
# keyboard modules are linked. No controller symbol is supplied by the stubs.
stubs=build/'os.asm'
stubs.write_text(' .module os\n .globl _read,_ioctl,_log_error\n .area _TEXT\n_read::\n_ioctl::\n_log_error::\n ret\n')
subprocess.run(['sdasz80','-o',str(build/'os.rel'),str(stubs)],check=True)
for name,(body,wanted) in cases.items():
    source=build/(name+'.c')
    source.write_text('#include <zgdk/input.h>\nvoid entry(void) { controller_init(); '+body+' }\n')
    subprocess.run(['sdcc','-mz80','--codeseg','TEXT','--opt-code-speed','-Iinclude',
                    '-I'+os.environ['ZOS_PATH']+'/sdk/sdcc/include','-c',str(source),'-o',str(build/(name+'.rel'))],check=True)
    subprocess.run(['sdldz80','-n','-i','-m','-w','-b','_TEXT=0x2000','-b','_DATA=0x8000',
                    '-k',str(library.parent),'-l',library.stem,str(build/name),str(build/(name+'.rel')),str(build/'os.rel')],check=True)
    text=(build/(name+'.map')).read_text()
    symbols=re.findall(r'^\s+[0-9A-F]{8}\s+(_\w+)\s',text,re.M)
    # ASxxxx truncates map symbols at 32 characters.
    expected={s[:32] for s in base|wanted}
    actual={s for s in symbols if s.startswith(('_controller_','_ctrl_'))}
    assert actual==expected,(name,sorted(actual-expected),sorted(expected-actual))
    for symbol in expected:
        assert symbols.count(symbol)==1,(name,symbol,'duplicated definition')
    assert not any(s in text for s in ('_PORT1_bits','_PORT2_bits','_MOUSE_x','_MOUSE_y'))
    if name in ('get_speed','set_speed'):
        ram=int(re.search(r'^\s+([0-9A-F]+)\s+l__DATA\s',text,re.M)[1],16)
        assert ram==(1 if name=='set_speed' else 0),(name,ram,'sensitivity state')
    size=int(re.search(r'^\s+([0-9A-F]+)\s+l__TEXT\s',text,re.M)[1],16)
    print(f'{name}: {size} linked code bytes; exact capability set, shared helpers once')

# Reproduce the old transitive link costs with the preserved baseline archive.
for name,body in {
    'pad': 'controller_init(); controller_read_port(SNES_PORT2);',
    'mouse': 'controller_init(); controller_read_port(SNES_PORT2); controller_read_mouse(SNES_PORT2);',
    'init': 'controller_init();',
}.items():
    source=build/('before_'+name+'.c')
    reference=Path('tests/controller/reference/reference.h').resolve()
    source.write_text('#include "'+str(reference)+'"\nvoid entry(void) { '+body+' }\n')
    subprocess.run(['sdcc','-mz80','--codeseg','TEXT','--opt-code-speed','-Iinclude',
                    '-I'+os.environ['ZOS_PATH']+'/sdk/sdcc/include','-c',str(source),'-o',str(source.with_suffix('.rel'))],check=True)
    output=source.with_suffix('')
    subprocess.run(['sdldz80','-n','-i','-m','-w','-b','_TEXT=0x2000','-b','_DATA=0x8000',
                    '-k',str(build.parent),'-l','reference',str(output),str(source.with_suffix('.rel'))],check=True)
    text=output.with_suffix('.map').read_text()
    size=int(re.search(r'^\s+([0-9A-F]+)\s+l__TEXT\s',text,re.M)[1],16)
    print(f'C baseline {name}: {size} linked code bytes')
