/* Registration and outside policy need neither a tilemap nor a row cache. */
#include <zgdk/tilemap/flags.h>
#if defined(TEST_REFRESH) || defined(TEST_AT) || defined(TEST_AT_TILE)
#include <zgdk/tilemap.h>
TILEFLAGS_DEFINE_CACHE(1);
Tilemap *_TILEMAP;
#endif

void main(void) {
#if defined(TEST_REGISTER)
    tileflags_register(NULL);
#elif defined(TEST_SET_OUTSIDE)
    tileflags_set_outside(TF_NONE);
#elif defined(TEST_REFRESH)
    tileflags_refresh();
#elif defined(TEST_AT)
    tileflags_at(0, 0);
#elif defined(TEST_AT_TILE)
    tileflags_at_tile(0, 0);
#endif
}
