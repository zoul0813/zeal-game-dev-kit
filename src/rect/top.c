#include "zgdk/rect.h"

const uint16_t rect_top(Rect *rect) {
  return rect->y - rect->h;
}
