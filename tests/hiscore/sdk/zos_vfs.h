#pragma once
#include <stdint.h>
#include <zos_errors.h>
typedef int8_t zos_dev_t;
#define O_RDONLY 0
#define O_WRONLY 1
#define O_CREAT 16
#define O_TRUNC 4
#define open hiscore_test_open
#define close hiscore_test_close
#define read hiscore_test_read
#define write hiscore_test_write
zos_dev_t open(const char *, uint8_t);
zos_err_t close(zos_dev_t);
zos_err_t read(zos_dev_t, void *, uint16_t *);
zos_err_t write(zos_dev_t, const void *, uint16_t *);
