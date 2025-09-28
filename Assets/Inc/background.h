#ifndef INC_BACKGROUND_H_
#define INC_BACKGROUND_H_

#include <stdint.h>

extern const struct {
  unsigned int   width;
  unsigned int   height;
  unsigned int   bytes_per_pixel;
  const char    *comment;
  const uint8_t  pixel_data[320 * 240 * 3];
} background_map;

#endif /* INC_BACKGROUND_H_ */
