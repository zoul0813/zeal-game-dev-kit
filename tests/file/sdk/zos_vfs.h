#pragma once
#include <stdint.h>
#include <zos_errors.h>
typedef int8_t zos_dev_t;
zos_err_t read(zos_dev_t dev, void *buf, uint16_t *size);
