#include "zgdk/rect.h"

const uint16_t rect_left(Rect *rect) {
  return rect->x - rect->w;
}
