#ifndef INC_SHADOW_H_
#define INC_SHADOW_H_

#include <stdint.h>

extern const struct {
  unsigned int   width;
  unsigned int   height;
  unsigned int   bytes_per_pixel;
  const char    *comment;
  const uint8_t  pixel_data[68 * 68 * 3];
} shadow_map;

#endif /* INC_SHADOW_H_ */
