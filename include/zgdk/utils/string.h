#include <stdint.h>
#include <zos_vfs.h>

#ifndef UTILS_STRING_H
#define UTILS_STRING_H

#define printf  zgdk_printf
#define itoa    zgdk_itoa
#define fprintf zgdk_fprintf
#define sprintf zgdk_sprintf

/**
 * @brief Converts an integer `num` to a string `str` representation in the specified `base`.
 *
 * @param num The integer to convert.
 * @param str A buffer to store the resulting string.
 * @param base The base to use for conversion (e.g., 10 for decimal, 16 for hexadecimal).
 * @param alpha A character indicating whether hexadecimal digits should be in lowercase
 *              ('a') or uppercase ('A').
 */
void zgdk_itoa(int num, char* str, uint8_t base, char alpha);

/**
 * @brief Formats and prints output to the standard output (typically the screen).
 *
 * @param format A string that specifies how subsequent arguments are formatted and displayed.
 *               Supports various format specifiers.
 * @param ... A variable list of arguments that will be formatted according to the format string.
 */
void zgdk_printf(const char* format, ...);

/**
 * @brief Formats and prints output to a specified device (e.g., screen or file).
 *
 * @param dev The device where the output will be written (e.g., `DEV_STDOUT` for the screen).
 * @param format A string that specifies how subsequent arguments are formatted and displayed.
 *               Supports various format specifiers.
 * @param ... A variable list of arguments that will be formatted according to the format string.
 */
void zgdk_fprintf(zos_dev_t dev, const char* format, ...);

/**
 * @brief Formats and prints output to a specified device (e.g., screen or file).
 *
 * @param buffer The buffer to write the formatted string into
 * @param format A string that specifies how subsequent arguments are formatted and displayed.
 *               Supports various format specifiers.
 * @param ... A variable list of arguments that will be formatted according to the format string.
 */
int zgdk_sprintf(char *buffer, const char* format, ...);

#endif