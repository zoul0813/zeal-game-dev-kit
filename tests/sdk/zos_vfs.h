#pragma once
#include <stdint.h>
#include "zos_errors.h"
typedef int8_t zos_dev_t;

#define O_RDONLY 1
#define O_WRONLY 2
#define O_CREAT 4
#define O_TRUNC 8
#define write test_vfs_write
#define open test_vfs_open
#define close test_vfs_close
zos_err_t test_vfs_write(zos_dev_t,const void*,uint16_t*);
zos_dev_t test_vfs_open(const char*,uint8_t);
zos_err_t test_vfs_close(zos_dev_t);
