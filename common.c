#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void make_msg_header(msg_header_t* header, int16_t op, const char* filename) {
  int32_t sz_fn = strlen(filename);
  header->data = calloc(6 + sz_fn, 1);
  //would sizeof(int16_t) be clearer?
  memcpy(header->data, &op, 2); 
  memcpy(header->data + 2, &sz_fn , 4);
  header->data[0] = op;
  header->data[2] = sz_fn;

  memcpy(&(header->data[6]), filename, sz_fn);
  header->size = 6 + sz_fn;
}

void change_opcode(msg_header_t* header, int16_t op) {
  memcpy(header->data, &op, 2);
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
  int32_t sz_file = (int32_t)ftell(fd);
  fseek(fd, 0L, SEEK_SET);

  header->data = malloc(4 + sz_file);

  if(header->data == NULL) {
    fprintf(stderr, "Out of memory\n");
    exit(EXIT_FAILURE);
  }

  header->size = 4 + sz_file;
  header->data[0] = sz_file;
  
  //fill the buffer
  if(sz_file != (int32_t)fread(header->data + 4, 1, sz_file, fd)) {
    fprintf(stderr, "File buffer borked\n");
    exit(EXIT_FAILURE);
  }

}
void free_data_header(data_header_t* header) {
  free(header->data);
}
