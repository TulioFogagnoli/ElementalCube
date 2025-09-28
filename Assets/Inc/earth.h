#ifndef INC_EARTH_H_
#define INC_EARTH_H_

#include <stdint.h>

extern const struct {
  unsigned int   width;
  unsigned int   height;
  unsigned int   bytes_per_pixel;
  const char    *comment;
  const uint8_t  pixel_data[68 * 68 * 3];
} earth_map;

#endif /* INC_EARTH_H_ */
