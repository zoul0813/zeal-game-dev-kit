#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <zos_vfs.h>
#include <zos_keyboard.h>
#include "zgdk/input.h"

static uint8_t fifo[256];
static uint16_t available, position, chunk_size;
static unsigned read_calls, pad_reads, logs;
static zos_err_t read_error, mode_error, pad_error, flush_error;
static uint16_t pad_report;
uint16_t PORT1_bits, PORT2_bits;
int8_t MOUSE_x, MOUSE_y;

static void feed(const uint8_t* bytes, uint16_t size, uint16_t chunk)
{
    assert(size <= sizeof(fifo));
    if (size) {
        memcpy(fifo, bytes, size);
    }
    available = size;
    position = 0;
    chunk_size = chunk;
    read_calls = 0;
}

zos_err_t read(zos_dev_t dev, void* buffer, uint16_t* size)
{
    uint16_t count = available - position;
    assert(dev == DEV_STDIN);
    assert(((uintptr_t)buffer & 31) == 0);
    /* Capacity must be restored even after a short read. */
    assert(*size == 32);
    assert(++read_calls < 512);
    if (read_error) {
        return read_error;
    }
    if (count > chunk_size) {
        count = chunk_size;
    }
    if (count > *size) {
        count = *size;
    }
    memcpy(buffer, fifo + position, count);
    position += count;
    *size = count;
    return ERR_SUCCESS;
}

zos_err_t ioctl(zos_dev_t dev, uint8_t command, void* argument)
{
    assert(dev == DEV_STDIN && command == KB_CMD_SET_MODE);
    assert((uintptr_t)argument == (KB_READ_NON_BLOCK | KB_MODE_RAW));
    return mode_error;
}

void log_error(const char* message, uint8_t error)
{
    assert(message && error == read_error);
    logs++;
}

zos_err_t controller_init(void)
{
    return pad_error;
}

zos_err_t controller_flush(void)
{
    PORT1_bits = PORT2_bits = 0;
    MOUSE_x = MOUSE_y = 0;
    return flush_error;
}

uint16_t controller_read_port(uint8_t port)
{
    assert(port == SNES_PORT1);
    pad_reads++;
    return PORT1_bits = pad_report;
}

static void reset(void)
{
    read_error = mode_error = pad_error = flush_error = 0;
    pad_report = 0;
    pad_reads = logs = 0;
    feed(NULL, 0, 32);
    assert(input_init(1) == ERR_SUCCESS);
}

static uint16_t event(uint8_t code, uint8_t releasing)
{
    uint8_t bytes[] = {KB_RELEASED, code};
    feed(bytes + !releasing, releasing ? 2 : 1, 32);
    return keyboard_read();
}

static void check_mappings(void)
{
    static const uint8_t codes[] = {
        KB_UP_ARROW, KB_DOWN_ARROW, KB_LEFT_ARROW, KB_RIGHT_ARROW,
        KB_KEY_ENTER, KB_RIGHT_SHIFT, KB_KEY_QUOTE, KB_KEY_Z,
        KB_KEY_SPACE, KB_KEY_X, KB_KEY_A, KB_KEY_S, KB_KEY_Q, KB_KEY_W
    };
    static const uint16_t buttons[] = {
        BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT,
        BUTTON_START, BUTTON_SELECT, BUTTON_SELECT, BUTTON_B,
        BUTTON_B, BUTTON_A, BUTTON_Y, BUTTON_X, BUTTON_L, BUTTON_R
    };
    reset();
    for (unsigned i = 0; i < sizeof(codes); i++) {
        assert(event(codes[i], 0) == buttons[i]);
        assert(event(codes[i], 1) == 0);
    }
    assert(event(KB_KEY_Z, 0) == BUTTON_B);
    assert(event(KB_KEY_SPACE, 0) == BUTTON_B);
    assert(event(KB_KEY_Z, 1) == BUTTON_B);
    assert(event(KB_KEY_SPACE, 1) == 0);
    assert(event(KB_KEY_SPACE, 0) == BUTTON_B);
    assert(event(KB_KEY_Z, 0) == BUTTON_B);
    assert(event(KB_KEY_SPACE, 1) == BUTTON_B);
    assert(event(KB_KEY_Z, 1) == 0);
    assert(event(KB_RIGHT_SHIFT, 0) == BUTTON_SELECT);
    assert(event(KB_KEY_QUOTE, 0) == BUTTON_SELECT);
    assert(event(KB_KEY_QUOTE, 1) == BUTTON_SELECT);
    assert(event(KB_RIGHT_SHIFT, 1) == 0);
}

static void check_directions(void)
{
    static const uint8_t pairs[][2] = {
        {KB_LEFT_ARROW, KB_RIGHT_ARROW}, {KB_RIGHT_ARROW, KB_LEFT_ARROW},
        {KB_UP_ARROW, KB_DOWN_ARROW}, {KB_DOWN_ARROW, KB_UP_ARROW}
    };
    static const uint16_t buttons[][2] = {
        {BUTTON_LEFT, BUTTON_RIGHT}, {BUTTON_RIGHT, BUTTON_LEFT},
        {BUTTON_UP, BUTTON_DOWN}, {BUTTON_DOWN, BUTTON_UP}
    };
    for (unsigned i = 0; i < 4; i++) {
        reset();
        assert(event(pairs[i][0], 0) == buttons[i][0]);
        assert(event(pairs[i][1], 0) == buttons[i][1]);
        /* A repeat of the older key must not steal priority. */
        assert(event(pairs[i][0], 0) == buttons[i][1]);
        assert(event(pairs[i][1], 1) == buttons[i][0]);
        assert(event(pairs[i][0], 1) == 0);
        assert(event(pairs[i][0], 0) == buttons[i][0]);
        assert(event(pairs[i][1], 0) == buttons[i][1]);
        assert(event(pairs[i][0], 1) == buttons[i][1]);
        assert(event(pairs[i][1], 1) == 0);
    }
    reset();
    assert(event(KB_LEFT_ARROW, 0) == BUTTON_LEFT);
    assert(event(KB_UP_ARROW, 0) == (BUTTON_LEFT | BUTTON_UP));
    assert(event(KB_RIGHT_ARROW, 0) == (BUTTON_RIGHT | BUTTON_UP));
    assert(event(KB_DOWN_ARROW, 0) == (BUTTON_RIGHT | BUTTON_DOWN));
    assert(event(KB_RIGHT_ARROW, 1) == (BUTTON_LEFT | BUTTON_DOWN));
    assert(event(KB_DOWN_ARROW, 1) == (BUTTON_LEFT | BUTTON_UP));
}

static void check_stream(void)
{
    uint8_t bytes[64];
    /* The SDK promises complete pairs; tolerate splits from other drivers too. */
    memset(bytes, 0xff, sizeof(bytes));
    bytes[0] = KB_KEY_X;
    bytes[31] = KB_RELEASED;
    bytes[32] = KB_KEY_X;
    bytes[63] = KB_KEY_Q;
    for (uint16_t chunk = 1; chunk <= 32; chunk++) {
        reset();
        feed(bytes, sizeof(bytes), chunk);
        while (position < available) {
            keyboard_read();
        }
        assert(KEYBOARD_keys == BUTTON_L);
        assert(position == available);
    }
    reset();
    assert(event(KB_KEY_X, 0) == BUTTON_A);
    bytes[0] = KB_RELEASED;
    feed(bytes, 1, 32);
    assert(keyboard_read() == BUTTON_A);
    feed(NULL, 0, 32);
    assert(keyboard_read() == BUTTON_A);
    bytes[0] = KB_KEY_X;
    feed(bytes, 1, 32);
    assert(keyboard_read() == 0);
    /* An unmapped released key consumes its prefix too. */
    bytes[0] = KB_RELEASED;
    bytes[1] = 0xff;
    bytes[2] = KB_KEY_X;
    feed(bytes, 3, 32);
    assert(keyboard_read() == BUTTON_A);
}

static void check_poll_reads(void)
{
    static const uint16_t lengths[] = {0, 1, 31, 32, 33, 64, 65};
    static const unsigned calls[] = {1, 1, 1, 2, 2, 3, 3};
    uint8_t bytes[65];
    memset(bytes, KB_KEY_X, sizeof(bytes));
    for (unsigned i = 0; i < sizeof(lengths) / sizeof(lengths[0]); i++) {
        reset();
        feed(bytes, lengths[i], 32);
        assert(keyboard_read() == (lengths[i] ? BUTTON_A : 0));
        assert(read_calls == calls[i] && position == available);
    }
    /* A short read can leave later input for the next frame's poll. */
    reset();
    bytes[0] = KB_KEY_Z;
    bytes[1] = KB_KEY_X;
    feed(bytes, 2, 1);
    assert(keyboard_read() == BUTTON_B && read_calls == 1 && position == 1);
    assert(keyboard_read() == (BUTTON_B | BUTTON_A));
    assert(read_calls == 2 && position == available);
}

static void check_keyboard_flush(void)
{
    uint8_t bytes[64];
    reset();
    assert(event(KB_LEFT_ARROW, 0) == BUTTON_LEFT);
    memset(bytes, KB_KEY_X, sizeof(bytes));
    bytes[63] = KB_RELEASED;
    feed(bytes, sizeof(bytes), 7);
    assert(keyboard_flush() == ERR_SUCCESS);
    assert(position == available && KEYBOARD_keys == 0);
    /* The pending break must survive flush, without restoring a held key. */
    bytes[0] = KB_KEY_X;
    feed(bytes, 1, 32);
    assert(keyboard_read() == 0);
    assert(event(KB_RIGHT_ARROW, 0) == BUTTON_RIGHT);
    assert(event(KB_RIGHT_ARROW, 1) == 0);
    assert(event(KB_KEY_X, 0) == BUTTON_A);
}

static void check_pads(void)
{
    reset();
    assert(event(KB_KEY_Z, 0) == BUTTON_B);
    for (uint16_t id = 0x1000; id; id += 0x1000) {
        pad_report = id | BUTTON_LEFT | BUTTON_START;
        assert(input_read() == BUTTON_B);
    }
    pad_report = 0xffff;
    assert(input_read() == BUTTON_B);
    pad_report = 0;
    assert(input_read() == BUTTON_B);
    pad_report = BUTTON_LEFT | BUTTON_START;
    assert(input_read() == (BUTTON_B | BUTTON_LEFT | BUTTON_START));
    pad_report = 0x0fff;
    assert(input_read() == 0x0fff);

    /* Snapshot the pad at flush time, even if no input_read preceded it. */
    reset();
    pad_report = BUTTON_START;
    input_flush();
    assert(PORT1_bits == 0 && input_read() == 0);
    assert(input_read() == 0);
    assert(controller_read() == BUTTON_START);
    assert(input_read() == 0);
    pad_report |= BUTTON_A;
    assert(input_read() == BUTTON_A);
    /* Invalid reports require every button to be seen released again. */
    pad_report = 0xffff;
    assert(input_read() == 0);
    input_flush();
    pad_report = BUTTON_START | BUTTON_A;
    assert(input_read() == 0);
    pad_report = BUTTON_START;
    assert(input_read() == 0);
    pad_report |= BUTTON_A;
    assert(input_read() == BUTTON_A);
    /* Keyboard Start remains independent of suppressed pad Start. */
    assert(event(KB_KEY_ENTER, 0) == BUTTON_START);
    assert(input_read() == (BUTTON_START | BUTTON_A));
    assert(event(KB_KEY_ENTER, 1) == 0);
    pad_report = BUTTON_A;
    assert(input_read() == BUTTON_A);
    pad_report |= BUTTON_START;
    assert(input_read() == (BUTTON_START | BUTTON_A));
    input_flush();
    pad_report = BUTTON_START;
    assert(input_read() == 0);
    pad_report = BUTTON_A;
    assert(input_read() == BUTTON_A);
    /* Low-level reads remain raw and bypass high-level suppression. */
    assert(controller_read() == BUTTON_A);
    assert(input_read() == BUTTON_A);

    reset();
    assert(input_init(0) == ERR_SUCCESS);
    pad_report = 0xffff;
    assert(input_read() == 0 && pad_reads == 0);
    input_flush();
    assert(pad_reads == 0);
    assert(input_init(2) == ERR_SUCCESS);
    pad_report = BUTTON_START;
    assert(input_read() == 0);
    pad_report = 0;
    assert(input_read() == 0);
    pad_report = BUTTON_START;
    assert(input_read() == BUTTON_START);
    input_flush();
    assert(input_init(1) == ERR_SUCCESS);
    assert(input_read() == 0);
}

static void check_startup(void)
{
    for (uint16_t button = 1; button <= BUTTON_R; button <<= 1) {
        reset();
        pad_report = button;
        assert(input_read() == 0);
        assert(input_read() == 0);
        assert(controller_read() == button);
        pad_report = 0;
        assert(input_read() == 0);
        pad_report = button;
        assert(input_read() == button);
    }
    reset();
    pad_report = 0x0fff;
    assert(input_read() == 0);
    pad_report = 0;
    assert(input_read() == 0);
    pad_report = 0x0fff;
    assert(input_read() == 0x0fff);

    reset();
    pad_report = BUTTON_LEFT;
    assert(event(KB_KEY_X, 0) == BUTTON_A);
    assert(input_read() == BUTTON_A);
    pad_report |= BUTTON_START;
    assert(input_read() == (BUTTON_A | BUTTON_START));
    assert(event(KB_KEY_X, 1) == 0);
    pad_report = 0;
    assert(input_read() == 0);
    pad_report = BUTTON_LEFT;
    assert(input_read() == BUTTON_LEFT);
}

static void check_reconnect(void)
{
    static const uint16_t invalid[] = {0xffff, 0x8000, 0x8100, 0x1000};
    for (unsigned i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++) {
        for (unsigned flushing = 0; flushing < 2; flushing++) {
            reset();
            assert(input_read() == 0);
            pad_report = BUTTON_LEFT;
            assert(input_read() == BUTTON_LEFT);
            pad_report = invalid[i];
            if (flushing) {
                input_flush();
            } else {
                assert(input_read() == 0);
            }
            pad_report = BUTTON_LEFT | BUTTON_START;
            assert(input_read() == 0);
            assert(input_read() == 0);
            /* Each button becomes usable independently when released. */
            pad_report = BUTTON_LEFT;
            assert(input_read() == 0);
            pad_report |= BUTTON_START;
            assert(input_read() == BUTTON_START);
            pad_report = BUTTON_START;
            assert(input_read() == BUTTON_START);
            pad_report |= BUTTON_LEFT;
            assert(input_read() == (BUTTON_LEFT | BUTTON_START));
        }
    }
}

static void check_errors(void)
{
    uint8_t byte = KB_RELEASED;
    reset();
    assert(event(KB_KEY_Z, 0) == BUTTON_B);
    feed(&byte, 1, 32);
    assert(keyboard_read() == BUTTON_B);
    read_error = ERR_FAILURE;
    assert(keyboard_read() == 0 && logs == 1);
    assert(keyboard_flush() == ERR_FAILURE && KEYBOARD_keys == 0);
    read_error = 0;
    assert(event(KB_KEY_X, 0) == BUTTON_A);
    reset();
    mode_error = ERR_FAILURE;
    assert(input_init(1) == ERR_FAILURE);
    pad_report = BUTTON_START;
    assert(input_read() == 0 && pad_reads == 0);
    reset();
    pad_error = ERR_FAILURE;
    assert(input_init(1) == ERR_BAD_MODE);
    pad_report = BUTTON_START;
    assert(input_read() == 0 && pad_reads == 0);
    reset();
    flush_error = ERR_FAILURE;
    assert(input_init(1) == ERR_ENTRY_CORRUPTED);
    assert(input_read() == 0 && pad_reads == 0);
}

int main(void)
{
    check_mappings();
    check_directions();
    check_stream();
    check_poll_reads();
    check_keyboard_flush();
    check_pads();
    check_startup();
    check_reconnect();
    check_errors();
    puts("Input regression checks passed");
    return 0;
}
