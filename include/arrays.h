#ifndef ARRAYS_H
#define ARRAYS_H

/**
 * Returns the dimensions/length of the array
 */
#define DIM(a) ( sizeof(a) / sizeof(*a) )

/** EXAMPLE
 *     FOR_EACH(NOTES) {
 *         uint8_t o = 2 - current->octave;
 *         uint16_t freq = NOTE_TO_FREQ(current->note);
 *         freq = freq >> (2 * o);
 *         current->freq = freq;
 *     }
 */
#define FOR_EACH(array) for (__typeof(*array) *current = array; current < array + DIM(array); current++)



#endif