/**
 * Generate a pulse on the LATCH pin, CLOCK must remain high during this process
 * Thanks to the preconfigured registers, this takes 24 T-States (2.4 microseconds @ 10MHz)
 */
#define LATCH_ONCE()                                       \
    do {                                                   \
        IO_PIO_DATA_A = (1 << IO_CLOCK) | (1 << IO_LATCH); \
        IO_PIO_DATA_A = (1 << IO_CLOCK);                   \
    } while (0)

/**
 * Pulse the clock
 */
#define CLOCK_ONCE()                   \
    do {                               \
        IO_PIO_DATA_A = 0;             \
        IO_PIO_DATA_A = 1 << IO_CLOCK; \
    } while (0)

/*
 * 16 t-states, 1.6us
 */
#define NOP()       \
    __asm__("NOP\n" \
            "NOP\n" \
            "NOP\n" \
            "NOP\n")

#define NOP_WAIT(times) \
    for(uint8_t mouse_wait = times; mouse_wait > 0; mouse_wait--) { \
        __asm__("NOP\n"); \
    }


#define GET_DATA(port) (IO_PIO_DATA_A & (1 << port))

#define IO_PIO_DISABLE_INT 0x03
#define IO_PIO_BITCTRL     0xcf

#define IO_DATA1 0
#define IO_DATA2 1
#define IO_LATCH 2
#define IO_CLOCK 3
