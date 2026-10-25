#pragma once
#include "zos_errors.h"
typedef int8_t zos_dev_t;
#define DEV_STDIN 1
#define read input_test_read
#define ioctl input_test_ioctl
zos_err_t read(zos_dev_t dev, void* buffer, uint16_t* size);
zos_err_t ioctl(zos_dev_t dev, uint8_t command, void* argument);
