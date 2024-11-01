#include <string.h>
#include "types.h"
#include "misc.h"
#include "tiles.h"

uint16_t tilemap_offset_x = 0;
uint16_t tilemap_offset_y = 0;

bool tile_get(Rect *src, Tile *tile) {

  uint16_t srcx = src->x + (src->w / 2);
  uint16_t srcy = src->y + (src->h / 2);

  int8_t tx = (srcx >> 4) - 1; // tilemap x
  int8_t ty = (srcy >> 4) - 1; // tilemap y
  if(tx < 0 || ty < 0) return false;

  // tile pixel coords
  uint16_t tile_t = ty * TILE_HEIGHT;
  uint16_t tile_b = tile_t + TILE_HEIGHT;
  uint16_t tile_l = tx * TILE_WIDTH;
  uint16_t tile_r = tile_l + TILE_WIDTH;

  // transpose into "sprite space"
  tile->x = tx;
  tile->y = ty;

  tile->rect.x = tile_l + tilemap_offset_x + TILE_WIDTH;
  tile->rect.y = tile_t + tilemap_offset_y + TILE_HEIGHT;
  tile->rect.w = TILE_WIDTH;
  tile->rect.h = TILE_HEIGHT;
  return true;
}

Edge tile_collide(Rect *rect, Direction *direction, Tile *tile) {
  Tile target;
  bool found = tile_get(rect, &target);
  if(!found) return EdgeNone;

  const uint16_t target_t = rect_top(&target.rect) + TILE_HALF;
  const uint16_t target_b = rect_bottom(&target.rect) - TILE_HALF - 1;
  const uint16_t target_l = rect_left(&target.rect);
  const uint16_t target_r = rect_right(&target.rect);

  const uint16_t rect_t = rect_top(rect);
  const uint16_t rect_b = rect_bottom(rect);
  const uint16_t rect_l = rect_left(rect) - 1;
  const uint16_t rect_r = rect_right(rect) + 1;

  /**
   * [ ] right edge of src touches left edge of tile
   * [ ]left edge of src touches right edge of tile
   * [x] bottom edge of src touches top edge of tile
   * [x] top edge of src touches bottom edge of tile
   * [ ] src is "inside" tile ... ?
   */

  // (void)rect;
  (void)direction;
  // (void)tile;

  Edge edge = EdgeNone;
  if(rect_r <= target_l) edge |= EdgeLeft;
  else if(rect_l > target_r) edge |= EdgeRight;

  if(rect_b <= target_t) edge |= EdgeTop;
  else if(rect_t > target_b) edge |= EdgeBottom;

  memcpy(tile, &target, sizeof(Tile));
  return edge;
}