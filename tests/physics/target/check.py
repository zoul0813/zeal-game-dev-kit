from pathlib import Path
import sys
root = Path(sys.argv[1])
free = (root/'free.map').read_text()
collision = (root/'collision.map').read_text()
platformer = (root/'platformer.map').read_text()
for name in ('_body_step', '_tileflags_rows', '_tileflags_register', '_platformer_update', '_platformer_move'):
    assert name not in free, ('free body pulled in', name)
for name in ('_platformer_update', '_platformer_ladder', '_platformer_move'):
    assert name not in collision, ('collision pulled in', name)
assert '_body_init' in free
assert '_body_step' in collision and '_tileflags_rows' in collision
assert '_platformer_update' in platformer and '_platformer_move' in platformer
assert '_platformer_ladder' not in platformer, 'unused ladder code linked'
print('physics archive extraction: free bodies, collision and platformer remain independent')
