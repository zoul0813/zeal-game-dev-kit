#include "zgdk/utils/_strlen.h"

uint16_t zgdk_strlen(const char* str)
{
    uint16_t length = 0;
    while (str[length]) length++;
    return length;
}