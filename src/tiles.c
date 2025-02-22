#include <string.h>
#include "zgdk/types.h"
#include "zgdk/types.h"
#include "zgdk/rect.h"
#include "zgdk/tiles.h"
#include "zgdk/tilemap.h"


void tile_at(uint8_t tx, uint8_t ty, Tile *tile) {
  // tile pixel coords
  uint16_t tile_t = ty * TILE_HEIGHT;
  uint16_t tile_b = tile_t + TILE_HEIGHT;
  uint16_t tile_l = tx * TILE_WIDTH;
  uint16_t tile_r = tile_l + TILE_WIDTH;

  tile->x = tx;
  tile->y = ty;

  // transpose into "sprite space"
  tile->rect.x = tile_r + tilemap_scroll_get_x(0);
  tile->rect.y = tile_b + tilemap_scroll_get_y(0);
  tile->rect.w = TILE_WIDTH;
  tile->rect.h = TILE_HEIGHT;
}

bool tile_get(Rect *src, Tile *tile) {
  // center of point
  uint16_t srcx = src->x - (src->w / 2);
  uint16_t srcy = src->y - (src->h / 2);

  int8_t tx = (srcx >> 4); // tilemap x
  int8_t ty = (srcy >> 4); // tilemap y
  if(tx < 0 || ty < 0) return false;

  tile_at(tx, ty, tile);
  return true;
}

bool tile_get_edge(Rect* rect, Edge edge, Tile* tile) {
  uint16_t srcx = rect->x;
  uint16_t srcy = rect->y;
  // center of point
  switch(edge) {
    case EdgeRight:
      srcx += rect->w;
      break;
    case EdgeBottom:
      srcy += rect->h;
      break;
  }

  int8_t tx = (srcx >> 4); // tilemap x
  int8_t ty = (srcy >> 4); // tilemap y
  if(tx < 0 || ty < 0) return false;

  tile_at(tx, ty, tile);
  return true;
}

Edge tile_collide(Rect *rect, Direction *direction, Tile *tile) {
  // (void)rect;
  (void)direction; // TODO: use the direction to determine which tile to retrieve
  // (void)tile;


  Tile target_h; //, target_v;
  bool found_h; //, found_v;
  // if(direction->x > 0) {
  // } else if(direction->x < 0) {
  // }
  // if(direction->y > 0) {
  // } else if(direction->y < 0) {
  // }

  found_h = tile_get(rect, &target_h);
  if(!found_h) return EdgeNone;

  const uint16_t target_t = rect_top(&target_h.rect);
  const uint16_t target_b = rect_bottom(&target_h.rect);
  const uint16_t target_l = rect_left(&target_h.rect);
  const uint16_t target_r = rect_right(&target_h.rect);

  const uint16_t rect_t = rect_top(rect);
  const uint16_t rect_b = rect_bottom(rect);
  const uint16_t rect_l = rect_left(rect);
  const uint16_t rect_r = rect_right(rect);

  Edge edge = EdgeNone;

  if(rect_r > target_r && rect_l <= target_r) edge |= EdgeRight;
  else if(rect_l < target_l && rect_r >= target_l) edge |= EdgeLeft;

  if(rect_b > target_b && rect_t <= target_b) edge |= EdgeBottom;
  else if(rect_t < target_t && rect_b >= target_t) edge |= EdgeTop;

  // which target?
  memcpy(tile, &target_h, sizeof(Tile));
  // memcpy(tile, &target_v, sizeof(Tile));
  return edge;
}
