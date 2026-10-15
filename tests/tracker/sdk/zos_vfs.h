#pragma once
#include <stdint.h>
typedef uint8_t zos_err_t;
typedef int8_t zos_dev_t;
#define ERR_SUCCESS 0
#define ERR_FAILURE 1
#define ERR_INVALID_PARAMETER 6
#define ERR_INVALID_OFFSET 9
#define ERR_NO_MORE_MEMORY 22
#define ERR_ENTRY_CORRUPTED 25
#define O_RDONLY 1
#define O_WRONLY 2
#define O_CREAT 4
#define O_TRUNC 8
#define open tracker_test_open
#define close tracker_test_close
#define read tracker_test_read
#define write tracker_test_write
zos_dev_t open(const char*, uint8_t);
zos_err_t close(zos_dev_t);
zos_err_t read(zos_dev_t, void*, uint16_t*);
zos_err_t write(zos_dev_t, const void*, uint16_t*);
