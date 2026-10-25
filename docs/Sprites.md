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

```C
void draw(void) {
  gfx_wait_vblank(&vctx);
  sprites_render(&vctx);
  gfx_wait_end_vblank(&vctx);
}
```

Call `sprites_deregister()` to release all registrations before loading a new scene. Previously returned sprite pointers are no longer valid. Register the new scene's sprites, then call `sprites_render(ctx)` to update the display and remove old sprites. Render even if the new scene has no sprites.

Keep the arena alive until that render succeeds, retrying if needed, before releasing it or replacing it with `sprites_register_arena()`.
