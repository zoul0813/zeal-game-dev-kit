#ifndef UTILS_RAND_H
#define UTILS_RAND_H

/**
 * Generate a quick 8-bit random number using the Z80's R register
 *
 * @return Value of the R register, 8-bit pseudo random value
 */
char rand8_quick(void);

/**
 * Generate an 8-bit random number using a combination of an LFSR w/ Counter
 *
 * @return 8-bit random value
 */
char rand8(void);

#endif