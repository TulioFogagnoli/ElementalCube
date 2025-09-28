#ifndef INC_LIGHT30_H_
#define INC_LIGHT30_H_

#include <stdint.h>

extern const struct {
  unsigned int   width;
  unsigned int   height;
  unsigned int   bytes_per_pixel;
  const char    *comment;
  const uint8_t  pixel_data[30 * 30 * 3];
} light30_map;

#endif /* INC_LIGHT30_H_ */
