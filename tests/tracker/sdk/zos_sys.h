#pragma once
#include "zos_vfs.h"
#define __critical
zos_err_t palloc(uint8_t* page);
zos_err_t pfree(uint8_t page);
