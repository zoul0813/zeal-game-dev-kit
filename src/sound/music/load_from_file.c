#include <core.h>
#include <zos_errors.h>
#include <zos_vfs.h>
#include "zgdk/utils/log.h"
#include "private.h"
zos_err_t music_load_from_file(const char* path, Track *track) {
  // const char* filename = "B:/piano.ptz";
  zos_dev_t dev = open(path, O_RDONLY);
  if(dev < 0) {
    // failed to open
    log_str("Failed to load file: ", path);
    return -dev;
  }

  uint16_t position = 0;
  music_init(track);

  uint16_t size = sizeof(uint16_t);
  zos_err_t err = read(dev, &track->length, &size);
  if(err != ERR_SUCCESS) {
    log_error("Failed to load header,", err);
    return err;
  }

  size = sizeof(Record) * track->length;
  err = read(dev, track->records, &size);
  if(err != ERR_SUCCESS) {
    log_error("Failed to load records,", err);
    return err;
  }

  close(dev);

  _track = track;

  return ERR_SUCCESS;
}
