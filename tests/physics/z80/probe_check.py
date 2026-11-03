import random, re, sys, z80

OUT = sys.argv[1]
HALT_AT = 0xFF00

def symbols(v):
    syms = {}
    for line in open(f"{OUT}/h_{v}.map"):
        m = re.search(r"\b([0-9A-F]{8})\s+(_\w+)", line)
        if m: syms[m.group(2)] = int(m.group(1), 16)
    return syms

def machine(v):
    m = z80.Z80Machine()
    m.set_memory_block(0, open(f"{OUT}/h_{v}.bin", "rb").read())
    m.set_memory_block(HALT_AT, b"\x76")
    m.set_breakpoint(HALT_AT)
    return m, symbols(v)

def w16(m, a, v): m.set_memory_block(a, bytes([v & 255, (v >> 8) & 255]))

def setup(m, s, W, H, tiles, flags, outside):
    TILES, FLAGS = 0x8000, 0x9F00
    m.set_memory_block(TILES, bytes(tiles)); m.set_memory_block(FLAGS, bytes(flags))
    w16(m, s["__tf_tiles"], TILES); w16(m, s["__tf_flags"], FLAGS)
    m.set_memory_block(s["__tf_width"], bytes([W])); m.set_memory_block(s["__tf_height"], bytes([H]))
    m.set_memory_block(s["__tf_outside"], bytes([outside]))
    # Also accept pre-grid fixtures for before/after comparisons.
    rows = s.get("_tileflags_rows", s.get("__tf_rows", s.get("__tf_row")))
    for y in range(H): w16(m, rows + 2 * y, (TILES if "_tileflags_rows" in s or "__tf_rows" in s else 0) + y * W)

TICKS = [0]

def call_ret(m, addr, hl=0, de=0, stack=()):
    """sdcccall(1): first args in HL/DE, the rest on the stack (callee pops). Counts T-states."""
    sp = 0xFE00
    for v in reversed(stack): sp -= 2; w16(m, sp, v)
    sp -= 2; w16(m, sp, HALT_AT)    # return address
    m.sp = sp; m.hl = hl & 0xFFFF; m.de = de & 0xFFFF; m.pc = addr
    # ticks_to_stop saturates at zero. A budget of 1 counts instructions,
    # not their T-states; stop on the return breakpoint with budget remaining.
    budget = 1000000
    m.ticks_to_stop = budget
    while m.pc != HALT_AT:
        m.run()
        if not m.ticks_to_stop: raise RuntimeError("runaway")
    TICKS[0] += budget - m.ticks_to_stop
    assert m.sp == 0xFE00, f"stack not balanced: {m.sp:#x}"
    return m.a

def call(m, addr, a, b, c):
    return call_ret(m, addr, a, b, (c,))

def ref(W, H, tiles, flags, outside, fixed, lo, hi, row):
    lim_fixed, lim_span = (H, W) if row else (W, H)
    f0 = fixed >> 4
    if fixed < 0 or f0 >= lim_fixed: return outside
    f = 0; t0, t1 = lo >> 4, hi >> 4
    if lo < 0: f = outside; t0 = 0
    if t1 >= lim_span: f |= outside; t1 = lim_span - 1
    for t in range(t0, t1 + 1):
        f |= flags[tiles[(f0 * W + t) if row else (t * W + f0)]]
    return f

def rnd16(r):
    return r.choice([r.randint(-40, 40), r.randint(-300, 4500), r.randint(-32768, 32767), r.randint(0, 200)])

r = random.Random(1)
ms = {v: machine(v) for v in ("asm", "c")}
bad = 0
cases = 0
for trial in range(40):
    W, H = r.choice([(20, 15), (100, 30), (1, 1), (255, 6), (80, 40), (7, 3), (0, 5), (6, 0)])
    tiles = [r.randrange(256) for _ in range(W * H)]
    flags = [r.randrange(256) if r.random() < 0.3 else 0 for _ in range(256)]
    outside = r.choice([1, 0x80, 0])
    for v, (m, s) in ms.items(): setup(m, s, W, H, tiles, flags, outside)
    for _ in range(150):
        row = r.random() < 0.5
        fixed = rnd16(r); lo = rnd16(r)
        hi = lo + r.choice([0, 15, 31, 47, r.randint(-50, 400)]) if r.random() < 0.8 else rnd16(r)
        hi = max(-32768, min(32767, hi))
        want = ref(W, H, tiles, flags, outside, fixed, lo, hi, row)
        cases += 1
        for v, (m, s) in ms.items():
            fn = s["_t_row" if row else "_t_col"]
            before = m.ticks_to_stop
            got = call(m, fn, fixed, lo, hi)
            if got != want:
                bad += 1
                if bad < 10: print(f"MISMATCH {v} {'row' if row else 'col'} W={W} H={H} fixed={fixed} lo={lo} hi={hi} got={got:#x} want={want:#x}")

# full platformer_move(): the asm build and the C reference build must agree frame by frame
import struct
BODY, TILES = 0xA000, 0x8000
step_bad = 0; steps = 0; body_cycles = {"asm": 0, "c": 0}
for trial in range(60):
    W, H = r.choice([(20, 15), (40, 12), (8, 8)])
    tiles = [r.choice([0, 0, 0, 1, 2, 3, 4]) for _ in range(W * H)]
    flags = [0, 1, 2, 4, 8] + [0] * 251          # solid, one-way, ladder, hazard
    outside = r.choice([1, 0])
    cfg = struct.pack("<7h3B", r.randint(2, 12), r.randint(40, 120), -r.randint(40, 90), -r.randint(5, 30),
                      r.randint(1, 8), r.randint(1, 8), r.randint(10, 48), 4, 4, 0)
    x, y = FP_ = r.randint(-20, W * 16 + 10) * 16, r.randint(-20, H * 16 + 10) * 16
    body = struct.pack("<4h4B", x, y, r.randint(-60, 60), r.randint(-60, 60), r.choice([12, 14, 16]),
                       r.choice([14, 16, 30, 32]), r.choice([0, 0x20]), 0)
    for v, (m, s) in ms.items():
        setup(m, s, W, H, tiles, flags, outside)
        m.set_memory_block(s["_CFG"], cfg)
        m.set_memory_block(BODY, body)
        call_ret(m, s["_t_map"], TILES, 0, (W | (H << 8),))
    for f in range(40):
        out = {}
        for v, (m, s) in ms.items():
            if f % 10 == 5:   # nudge velocity like input would
                m.set_memory_block(BODY + 4, struct.pack("<h", (f * 7919 + trial * 31) % 97 - 48))
            TICKS[0] = 0
            call_ret(m, s["_t_move"], BODY)
            body_cycles[v] += TICKS[0]
            out[v] = bytes(m.memory[BODY:BODY + 12])
        steps += 1
        if out["asm"] != out["c"]:
            step_bad += 1
            if step_bad < 5: print("BODY MISMATCH", trial, f, out["asm"].hex(), out["c"].hex(), "start", body.hex(), "W,H,out", W, H, outside)
            break
print(f"platformer_move frames compared: {steps}, mismatches: {step_bad}; avg T-states per call: asm {body_cycles['asm'] // steps}, C {body_cycles['c'] // steps}")
bad += step_bad
print(f"probe cases: {cases}, mismatches: {bad - step_bad}")
sys.exit(1 if bad else 0)
