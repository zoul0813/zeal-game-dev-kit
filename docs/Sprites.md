# ZGDK: Sprites

ZGDK provides a Sprite Arena to help with allocating a large number of sprites and rendering them effeciently.

You can create and register a Sprite Arena by defining an array of `gfx_sprite` and calling `sprites_register_arena(sprites, sizeof(sprites));`

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
