#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <zos_vfs.h>
#include <zgdk/hiscore.h>
#include <zgdk/input.h>
#include <zgdk/tilemap.h>

static unsigned ui_calls, io_calls;
static unsigned char file[256];
static uint16_t file_size, cursor;
static zos_err_t io_error;

zos_dev_t open(const char *path, uint8_t flags) {
    assert(path && !strcmp(path, "scores.dat"));
    ++io_calls;
    cursor = 0;
    if (io_error) return -io_error;
    if (flags & O_TRUNC) file_size = 0;
    return 1;
}
zos_err_t close(zos_dev_t dev) { assert(dev == 1); return ERR_SUCCESS; }
zos_err_t write(zos_dev_t dev, const void *data, uint16_t *size) {
    assert(dev == 1 && cursor + *size <= sizeof(file));
    memcpy(file + cursor, data, *size);
    file_size = cursor += *size;
    return ERR_SUCCESS;
}
zos_err_t read(zos_dev_t dev, void *data, uint16_t *size) {
    assert(dev == 1);
    if (*size > file_size - cursor) *size = file_size - cursor;
    memcpy(data, file + cursor, *size);
    cursor += *size;
    return ERR_SUCCESS;
}
zos_err_t ensure_parent_dirs(const char *path) { assert(path); return ERR_SUCCESS; }
uint16_t input_read(void) { ++ui_calls; return BUTTON_B; }
void gfx_wait_vblank(gfx_context *ctx) { assert(ctx); ++ui_calls; }
void gfx_wait_end_vblank(gfx_context *ctx) { assert(ctx); ++ui_calls; }
void msleep(uint16_t delay) { assert(delay == 3000); ++ui_calls; }
void itoa_pad(uint16_t value, char *out, uint16_t base, char alpha, char pad, uint8_t width) {
    assert(base == 10 && alpha == 'A' && pad == '0' && width == 5);
    snprintf(out, 6, "%05u", (unsigned)value);
}
gfx_error tilemap_fill(gfx_context *ctx, uint8_t layer, uint8_t tile,
                      uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    assert(ctx && layer == 1 && tile == 0 && !x && !y && width == 40 && height == 30);
    ++ui_calls;
    return GFX_SUCCESS;
}
void hud_tilemap_nput_s_layer(gfx_context *ctx, const char *text, uint8_t len,
                         uint8_t layer, uint8_t x, uint8_t y) {
    assert(ctx && text && layer == 1 && x == 15 && y >= 3 && y < 13);
    assert(len == 10 || len == 1);
    if (len == 10) assert(text[3] == ' ' && text[4] == ' ');
    ++ui_calls;
}

int main(void) {
    static const highscore_t defaults[HISCORES_COUNT] = {
        {{'T','O','P'}, 900}, {{'X','Y','Z'}, 800},
        {{'A','B','C'}, 700}, {{'A','B','C'}, 500}
    };
    const char player[3] = {'A', 'B', 'C'}; /* No trailing NUL. */
    gfx_context ctx = {0};
    hiscore_config_t config = {&ctx, 40, 30, 1, 0, defaults, NULL};
    highscore_t snapshot[HISCORES_COUNT];
    uint8_t i;
    hiscore_init(&config);
    for (i = 0; i < HISCORES_COUNT; ++i) {
        assert(hiscore_get(i)->score == defaults[i].score);
        assert(!memcmp(hiscore_get(i)->initials, defaults[i].initials, 3));
    }
    assert(hiscore_get(0)->score == 900);
    assert(hiscore_get(HISCORES_COUNT) == NULL && hiscore_get(255) == NULL);
    assert(hiscore_find(player) == hiscore_get(2));
    assert(!hiscore_find("abc") && !hiscore_find("NON") && !hiscore_find(NULL));
    assert(!ui_calls && !io_calls); /* HUD queries perform no display/input/I/O. */
    assert(hiscore_add(950) == 0);
    assert(hiscore_get(0)->score == 950 && hiscore_find("AAA") == hiscore_get(0));
    assert(hiscore_find(player) == hiscore_get(3));
    for (i = 1; i < HISCORES_COUNT; ++i)
        assert(hiscore_get(i-1)->score >= hiscore_get(i)->score);
    {
        unsigned before = ui_calls;
        assert(hiscore_add(0) == -1 && ui_calls == before);
    }
    for (i = 0; i < HISCORES_COUNT; ++i) snapshot[i] = *hiscore_get(i);
    assert(hiscore_save("scores.dat") == ERR_SUCCESS);
    assert(file_size == 7 + sizeof(snapshot));
    assert(!memcmp(file, "ZHS", 3) && file[3] == HISCORE_VERSION);
    assert(!memcmp(file + 4, snapshot, sizeof(snapshot)));
    assert(!memcmp(file + 4 + sizeof(snapshot), "AAA", 3));
    config.default_scores = NULL;
    hiscore_init(&config);
    assert(!hiscore_get(0)->score && !hiscore_find(player));
    assert(hiscore_load("scores.dat") == ERR_SUCCESS);
    assert(hiscore_get(0)->score == 950 && hiscore_find(player)->score == 700);
    --file_size;
    assert(hiscore_load("scores.dat") == ERR_ENTRY_CORRUPTED);
    for (i = 0; i < HISCORES_COUNT; ++i)
        assert(!memcmp(hiscore_get(i), &snapshot[i], sizeof(highscore_t)));
    ++file_size;
    file[0] = 'X';
    assert(hiscore_load("scores.dat") == ERR_ENTRY_CORRUPTED);
    file[0] = 'Z';
    io_error = ERR_FAILURE;
    assert(hiscore_load("scores.dat") == ERR_FAILURE);
    assert(hiscore_save("scores.dat") == ERR_FAILURE);
    io_error = ERR_SUCCESS;
    config.path = "scores.dat";
    hiscore_init(&config); /* Existing automatic load contract survives the split. */
    assert(hiscore_get(0)->score == 950 && hiscore_find(player)->score == 700);
    puts("hiscores: ranked/player HUD reads, entry UI, persistence and failed-load preservation passed");
    return 0;
}
