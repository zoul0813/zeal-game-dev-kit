#include <stdint.h>
#include <zos_errors.h>
#include <zos_vfs.h>

#ifndef FILE_H
#define FILE_H

/**
 * @brief read() that splits at 16 KiB virtual page boundaries.
 *
 * ZOS read() buffers must not cross a virtual page (zos_vfs.h), so a plain read() into a buffer that
 * straddles a page boundary, or is bigger than a page, fails. This reads in pieces that each stay inside
 * one page, so any buffer in RAM works.
 *
 * Same contract as read(): *size is the bytes wanted, and on return the bytes read; it stops early, with
 * ERR_SUCCESS, at the end of the file. On an error, *size is the bytes read before it.
 */
zos_err_t file_read_safe(zos_dev_t dev, void *buffer, uint16_t *size);

#endif
