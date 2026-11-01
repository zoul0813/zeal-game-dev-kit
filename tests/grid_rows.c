#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <zgdk/utils/grid_rows.h>

int main(void) {
    uint8_t data[300 * 255], other[20 * 15];
    const uint8_t *rows[256];
    unsigned y, x;
    for (y = 0; y < sizeof(data); y++) data[y] = (uint8_t)y;
    rows[0] = data;
    grid_rows_init(rows, NULL, 0, 0);
    assert(rows[0] == data);

    /* Padded rows, more than 255 bytes per row and all 255 addressable rows.
     * Check every byte, including IDs above 127, against flat addressing. */
    rows[255] = other;
    grid_rows_init(rows, data, 300, 255);
    for (y = 0; y < 255; y++) {
        assert(rows[y] == data + y * 300);
        for (x = 0; x < 300; x++) assert(rows[y][x] == data[y * 300 + x]);
    }
    assert(rows[255] == other);

    /* Rebinding a table changes only its addresses; source edits remain visible. */
    grid_rows_init(rows, other, 20, 15);
    other[2 * 20 + 3] = 0xf1;
    assert(rows[2][3] == 0xf1);
    assert(rows[15] == data + 15 * 300);
    grid_rows_init(rows, data, 0, 2);
    assert(rows[0] == data && rows[1] == data);
    puts("grid rows: all passed");
    return 0;
}
