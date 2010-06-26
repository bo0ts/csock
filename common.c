#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

const char* op_tbl[] = { "", "REQUEST_DOWN", "REPLY_DOWN", "REQUEST_UP", "REPLY_UP", "ERROR" };

void make_msg_header(msg_header_t* header, uint16_t op, const char* filename) {
  uint32_t sz_fn = strlen(filename);
  header->data = calloc(2 + 4 + sz_fn, 1); //opcode + fn_len + sz_fn
  //would sizeof(int16_t) be clearer?
  uint16_t op_net = htons(op);
  uint32_t sz_fn_net = htonl(sz_fn);
  memcpy(header->data, &op_net, 2); 
  memcpy(header->data + 2, &sz_fn_net , 4);
  memcpy(header->data + 6, filename, sz_fn);
  header->size = 6 + sz_fn;
}

void change_opcode(msg_header_t* header, uint16_t op) {
  uint16_t op_net = htons(op);
  memcpy(header->data, &op_net, 2);
}

void free_msg_header(msg_header_t* header) {
  free(header->data);
}

void make_data_header(data_header_t* header, const char* filename) {
  FILE* fd = fopen(filename, "rb"); //open binary read file

  if(!fd) {
    fprintf(stderr, "File error\n");
    exit(EXIT_FAILURE);
  }

  fseek(fd, 0L, SEEK_END);
  uint32_t sz_file = (uint32_t)ftell(fd);
  fseek(fd, 0L, SEEK_SET);

  header->data = malloc(4 + sz_file);

  if(header->data == NULL) {
    fprintf(stderr, "Out of memory\n");
    exit(EXIT_FAILURE);
  }

  header->size = 4 + sz_file;
  uint32_t sz_file_net = htonl(sz_file);
  memcpy(header->data, &sz_file_net, 4);
  
  //fill the buffer
  if(sz_file != (uint32_t)fread(header->data + 4, 1, sz_file, fd)) {
    fprintf(stderr, "File buffer borked\n");
    exit(EXIT_FAILURE);
  }

}
void free_data_header(data_header_t* header) {
  free(header->data);
}
