#include "common.h"
#include "string.h"

void make_header(header_t* header, int16_t op, int32_t len) {
  memset(header->data, 0, 6); //neccessary?
  header->data[0] = op;
  header->data[2] = len;
}
