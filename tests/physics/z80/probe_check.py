import random, re, struct, sys, z80

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
    syms = symbols(v)
    # There is no CRT: initialize C globals as SDCC's startup would.
    text = open(f"{OUT}/h_{v}.map").read()
    areas = {}
    for name in ("_DATA", "_INITIALIZER", "_INITIALIZED"):
        match = re.search(rf"^\s*{name}\s+([0-9A-F]+)\s+([0-9A-F]+)", text, re.M)
        if match: areas[name] = tuple(int(x, 16) for x in match.groups())
    if "_DATA" in areas:
        target, size = areas["_DATA"]
        m.set_memory_block(target, bytes(size))
    if "_INITIALIZER" in areas and "_INITIALIZED" in areas:
        source, size = areas["_INITIALIZER"]
        target, _ = areas["_INITIALIZED"]
        m.set_memory_block(target, bytes(m.memory[source:source + size]))
    return m, syms

def w16(m, a, v): m.set_memory_block(a, bytes([v & 255, (v >> 8) & 255]))

def setup(m, s, W, H, tiles, flags, outside):
    TILES, FLAGS = 0x8000, 0x9F00
    m.set_memory_block(TILES, bytes(tiles)); m.set_memory_block(FLAGS, bytes(flags))
    w16(m, s["__tf_tiles"], TILES); w16(m, s["__tf_flags"], FLAGS)
    m.set_memory_block(s["__tf_width"], bytes([W])); m.set_memory_block(s["__tf_height"], bytes([H]))
    m.set_memory_block(s["__tf_outside"], bytes([outside]))
    for y in range(H): w16(m, s["_tileflags_rows"] + 2 * y, TILES + y * W)

def call_ret(m, addr, hl=0, de=0, stack=(), budget=1000000):
    """sdcccall(1): first args in HL/DE, the rest on the stack (callee pops). Checks stack cleanup."""
    sp = 0xFE00
    for v in reversed(stack): sp -= 2; w16(m, sp, v)
    sp -= 2; w16(m, sp, HALT_AT)    # return address
    m.sp = sp; m.hl = hl & 0xFFFF; m.de = de & 0xFFFF; m.pc = addr
    # ticks_to_stop saturates at zero. A budget of 1 counts instructions,
    # not their T-states; stop on the return breakpoint with budget remaining.
    m.ticks_to_stop = budget
    while m.pc != HALT_AT:
        m.run()
        if not m.ticks_to_stop: raise RuntimeError("runaway")
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
m, syms = machine("probes")
bad = 0
cases = 0
for trial in range(40):
    W, H = r.choice([(20, 15), (100, 30), (1, 1), (255, 6), (80, 40), (7, 3), (0, 5), (6, 0)])
    tiles = [r.randrange(256) for _ in range(W * H)]
    flags = [r.randrange(256) if r.random() < 0.3 else 0 for _ in range(256)]
    outside = r.choice([1, 0x80, 0])
    setup(m, syms, W, H, tiles, flags, outside)
    for _ in range(150):
        row = r.random() < 0.5
        fixed = rnd16(r); lo = rnd16(r)
        hi = lo + r.choice([0, 15, 31, 47, r.randint(-50, 400)]) if r.random() < 0.8 else rnd16(r)
        hi = max(-32768, min(32767, hi))
        want = ref(W, H, tiles, flags, outside, fixed, lo, hi, row)
        cases += 1
        fn = syms["_t_row" if row else "_t_col"]
        got = call(m, fn, fixed, lo, hi)
        if got != want:
            bad += 1
            if bad < 10: print(f"MISMATCH {'row' if row else 'col'} W={W} H={H} fixed={fixed} lo={lo} hi={hi} got={got:#x} want={want:#x}")

# Fixed-point edges: signed positions, fractional pixels and all body dimensions.
BODY = 0xA000
for _ in range(2000):
    x, y = r.randint(-32768, 32767), r.randint(-32768, 32767)
    w, h = r.randint(1, 255), r.randint(1, 255)
    m.set_memory_block(BODY, struct.pack("<4h4B", x, y, 0, 0, w, h, 0, 0))
    call_ret(m, syms["_t_edges"], BODY)
    start = syms["_TE"]
    got = struct.unpack("<4h", bytes(m.memory[start:start + 8]))
    want = (x >> 4, (x >> 4) + w - 1, y >> 4, (y >> 4) + h - 1)
    assert got == want, (got, want)
print("fixed-point edge cases: 2000 passed")

# Existing collision/controller scenarios run against the linked production assembly.
for test in ("platformer", "tileflags"):
    m, syms = machine(test)
    call_ret(m, syms["_main"], budget=100000000)
    assert m.de == 0, (test, "behavior regression failed", m.de)
    print(f"{test} behavior scenarios: passed")
print(f"probe cases: {cases}, mismatches: {bad}")
sys.exit(1 if bad else 0)
