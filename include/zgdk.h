#include <stdint.h>
#include "types.h"
#include "input.h"
#include "sounds.h"
#include "tiles.h"
#include "ascii.h"
#include "utils.h"



#define TILE_SIZE           (16 * 16)
#define SPRITE_HEIGHT       16
#define SPRITE_WIDTH        16
#define SCROLL_WIDTH        1280
#define SCROLL_HEIGHT       640

#define DIRECTION_LEFT      (-1)
#define DIRECTION_RIGHT     (1)
#define DIRECTION_UP        (-1)
#define DIRECTION_DOWN      (1)