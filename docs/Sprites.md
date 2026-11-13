# ZGDK: Sprites

ZGDK provides a Sprite Arena to help with allocating a large number of sprites and rendering them effeciently.

Create a Sprite Arena by defining an array of `gfx_sprite` and calling `sprites_register_arena(sprites, SPRITE_ARENA_SIZE);`. The size is the number of elements, not bytes, and must be 1–128. A null pointer or invalid size returns `GFX_INVALID_ARG`.

```C
#define SPRITE_ARENA_SIZE 64
static gfx_sprite sprite_arena[SPRITE_ARENA_SIZE];

gfx_context vctx;

void init(void) {
  zos_err_t err;

  gfx_enable_screen(0);

  err = gfx_initialize(SCREEN_MODE, &vctx);
  handle_error(err, "failed to init graphics", 1);

  err = load_palette(&vctx);
  handle_error(err, "failed to load palette", 1);

  gfx_tileset_options options = {
    .compression = TILESET_COMP_RLE,
  };
  err = load_tileset(&vctx, &options);
  handle_error(err, "failed to load tileset", 1);

  // >>> Register Sprite Arena <<<
  err = sprites_register_arena(sprite_arena, (uint8_t)SPRITE_ARENA_SIZE);
  handle_error(err, "Failed to initialize sprite arena", 1);

  gfx_enable_screen(1);
}
```

Once your arena is defined and registered, you can allocate sprites by calling `sprites_register(sprite);`

Registration returns a pointer into the arena, or `NULL` when the arena is full.

```C
zos_err_t player_init(void) {
  gfx_sprite sprite = {
    .x = 0,
    .y = 0,
    .tile = 32,
    .flags = SPRITE_NONE
  };
  player.sprite = sprites_register(sprite);
}
```

To render all the sprites in your Arena, you can call `sprites_render(ctx);` in your draw function.
`sprites_render()` and `sprites_render_range()` always use the ZVB SDK CPU
copies. Arena registration does not probe or prepare DMA, and CPU-only callers
do not link the DMA backend or reserve its descriptor cache. The SDK CPU path
retains the SDK's interrupt-masking behavior. Pending old-slot cleanup is
committed only after the upload succeeds.

```C
void draw(void) {
  gfx_wait_vblank(&vctx);
  sprites_render(&vctx);
  gfx_wait_end_vblank(&vctx);
}
```

## Uploading a range

Register frequently changing sprites first, then upload only the range that
changed. Arena indices are zero-based and map to the same hardware sprite slots:

```C
// After initialization or a scene change, upload all registered sprites.
sprites_render(&vctx);

// Each frame: upload entries 0 through 11, leaving other slots untouched.
sprites_render_range(&vctx, 0, 12);

// When entries 12 through 17 change, upload that range separately.
sprites_render_range(&vctx, 12, 6);
```

The CPU range API requires
a valid context and registered arena, with `start + count` no greater than the
number of registered sprites; invalid requests return `GFX_INVALID_ARG` without
an upload. A zero count performs no upload and succeeds when `start` is at or
before the end of the registrations.

Range calls do not change arena contents, registration order, or registered
pointers. They also do not hide sprites outside the requested range. If an active
prefix shrinks, set the removed sprites to their hidden state and upload those
slots too. After deregistration, use a successful full `sprites_render()` to
flush old-slot cleanup; range calls preserve that pending cleanup.

## Explicit DMA uploads

Use the DMA entry points only when your game opts into DMA sprite transfers:

```C
// Full upload, including pending cleanup after deregistration.
sprites_render_dma(&vctx);

// Upload only slots 0 through 11 using DMA.
sprites_render_range_dma(&vctx, 0, 12);
```

These calls share validation, arena indices, and cleanup tracking with the CPU
APIs, so they may be mixed. A successful full upload through either API flushes
pending old-slot cleanup; neither range API clears slots outside its range.
An empty or invalid DMA request does not probe or prepare DMA.

The first nonempty explicit DMA call probes DMA once using private RAM and
prepares reusable transfer descriptors. Arena replacement, capacity changes,
and source MMU mapping changes rebuild the plan on demand; changes to sprite
contents or registration counts reuse it. Only callers that link an explicit DMA
entry point reserve the descriptor cache.

DMA uploads go directly to physical sprite VRAM in transfers of at most 32 bytes,
without remapping CPU memory or disabling interrupts. Transfers split at source
MMU page boundaries, and descriptors fit within one mapped page. The peripheral
bank and DMA clock divider are preserved. Separate transfers release the bus
between chunks. At the default DMA clock divider (`0x56`), 32-byte chunks passed
335 keyboard-release timing cases in the previous Enigma emulator workload;
64- and 128-byte chunks each lost a release. Other timings require renewed
validation. Nonempty DMA uploads return `GFX_FAILURE` if DMA is unavailable;
they never fall back to CPU copies. Failed uploads preserve pending cleanup and
the rendered-slot high-water mark. Valid empty uploads succeed without probing
DMA. Games may choose to call the corresponding CPU API themselves after a DMA
failure. Existing games keep using CPU copies until they change their calls to
the explicit DMA APIs.

Call `sprites_deregister()` to release all registrations before loading a new scene. Previously returned sprite pointers are no longer valid. Register the new scene's sprites, then call `sprites_render(ctx)` to update the display and remove old sprites. Render even if the new scene has no sprites.

Keep the arena alive until that render succeeds, retrying if needed, before releasing it or replacing it with `sprites_register_arena()`.

`sprites_clear(ctx)` immediately hides all 128 hardware sprites, including tall
sprites, without requiring an arena. It preserves arena contents and registrations;
the next `sprites_render(ctx)` restores the registered sprites. It returns
`GFX_SUCCESS` after clearing every slot, or the first SDK error and stops. On
failure, earlier slots may already be hidden; retry the clear after resolving
the error.
