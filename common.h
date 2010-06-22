#ifndef _COMMON_H__
#define _COMMON_H__

#include <inttypes.h>

#define REQUEST_DOWN 0x01
#define REPLY_DOWN 0x02
#define REQUEST_UP 0x03
#define REPLY_UP 0x04
#define ERROR 0x05

typedef struct {
  void* data[6];
} header_t;

void make_header(header_t* header, int16_t op, int32_t len);

#endif //header guard
