#include <stdarg.h>

#include "zgdk/utils/_strlen.h"
#include "zgdk/utils/string.h"

// Helper function to convert an integer to a string
void zgdk_itoa(int num, char* str, uint8_t base, char alpha) {
    int i = 0;
    int is_negative = 0;

    // Handle 0 explicitly, otherwise empty string is printed
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Handle negative numbers only if base is 10
    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }

    // Process individual digits
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + alpha : rem + '0';
        num = num / base;
    }

    // Append negative sign for negative numbers
    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    // Reverse the string
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}

// Simplified version of printf() with width and alignment support
int zgdk__sprintf(char *buffer, const char* format, va_list args) {
    int i = 0;

    while (*format) {
        if (*format == '%' && *(format + 1)) {  // Check for a format specifier
            format++;  // Skip '%' character

            // Parse the width specifier (e.g., %12d or %-5s)
            int width = 0;
            int left_align = 0;

            // Check for negative width (left alignment)
            if (*format == '-') {
                left_align = 1;
                format++;  // Skip the '-' character
            }

            // Parse the width value (e.g., 12 in %12d)
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format - '0');
                format++;
            }

            uint8_t base = 10;
            char alpha = 'a';

            switch (*format) {
                case 's': {  // String
                    const char* str = va_arg(args, const char*);
                    int len = strlen(str);
                    int pad = (width > len) ? width - len : 0;

                    // Left-align if negative width, otherwise right-align
                    if (left_align) {
                        // Copy the string
                        while (*str) {
                            buffer[i++] = *str++;
                        }

                        // Pad with spaces after the string
                        while (pad--) {
                            buffer[i++] = ' ';
                        }
                    } else {
                        // Pad with spaces before the string
                        while (pad--) {
                            buffer[i++] = ' ';
                        }

                        // Copy the string
                        while (*str) {
                            buffer[i++] = *str++;
                        }
                    }
                    break;
                }
                case 'X': alpha = 'A'; // Upper Hex, fallthru
                case 'x': base = 16; // Lower Hex, fallthru
                case 'd': {  // Integer
                    int num = va_arg(args, int);
                    char num_str[20];
                    zgdk_itoa(num, num_str, base, alpha);  // Convert integer to string
                    int len = strlen(num_str);
                    int pad = (width > len) ? width - len : 0;

                    // Left-align if negative width, otherwise right-align
                    if (left_align) {
                        // Copy the number string
                        char* num_ptr = num_str;
                        while (*num_ptr) {
                            buffer[i++] = *num_ptr++;
                        }

                        // Pad with spaces after the number
                        while (pad--) {
                            buffer[i++] = ' ';
                        }
                    } else {
                        // Pad with spaces before the number
                        while (pad--) {
                            buffer[i++] = ' ';
                        }

                        // Copy the number string
                        char* num_ptr = num_str;
                        while (*num_ptr) {
                            buffer[i++] = *num_ptr++;
                        }
                    }
                    break;
                }
                default:
                    // Handle unknown format specifiers if needed
                    break;
            }
        } else {
            buffer[i++] = *format;  // Copy regular characters
        }
        format++;
    }

    // Null-terminate the buffer
    buffer[i] = '\0';
    return i;
}

int zgdk_sprintf(char *buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int size = zgdk__sprintf(buffer, format, args);
    va_end(args);
    return size;
}

void zgdk_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];  // Output buffer for formatting
    int size = zgdk__sprintf(buffer, format, args);
    write(DEV_STDOUT, buffer, &size);  // Assume NULL as the device argument
    va_end(args);
}

void zgdk_fprintf(zos_dev_t dev, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];  // Output buffer for formatting
    int size = zgdk__sprintf(buffer, format, args);
    write(dev, buffer, &size);  // Assume NULL as the device argument
    va_end(args);
}

