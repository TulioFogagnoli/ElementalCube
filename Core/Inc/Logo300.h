#ifndef INC_LOGO300_H_
#define INC_LOGO300_H_

#include <stdint.h>

extern const struct {
  unsigned int   width;
  unsigned int   height;
  unsigned int   bytes_per_pixel;
  const char    *comment;
  const uint8_t  pixel_data[300 * 300 * 3];
} Logo300_map;

#endif /* INC_LOGO300_H_ */
