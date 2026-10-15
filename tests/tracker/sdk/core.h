#pragma once
#include <stddef.h>
#include <string.h>
#define mem_set memset
#define CH_NULL '\0'
void* tracker_test_memcpy(void* dst, const void* src, size_t size);
#define mem_cpy tracker_test_memcpy
