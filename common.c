#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>


#define BUF_SIZE 255
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

int file_to_socket(int sock, const char* filename) {
  FILE* file = fopen(filename, "rb");
  if(!file) {
    fprintf(stderr, "File error\n");
    return -1;
  }

  //file size, no stat
  fseek(file, 0L, SEEK_END);
  uint32_t sz_file = (uint32_t)ftell(file);
  fseek(file, 0L, SEEK_SET);
  sz_file = htonl(sz_file);

  if(4 != write(sock, &sz_file, 4)) {
    fclose(file);
    return -1;
  }

  char buffer[BUF_SIZE];
  int run = 1;
  while(run) {
    size_t read;
    if((read = fread(buffer, 1, 255, file)) < BUF_SIZE)
      run = 0;
    
    if(read != (size_t)write(sock, buffer, read)) {
      fclose(file);
      return -1;
    }
  }
  return 0;
}

int socket_to_file(int sock, const char* filename) {
  uint32_t sz_file;
  if(4 != read(sock, &sz_file, 4)) {
    fprintf(stderr, "Read in socket to file failed!\n");
    return -1;
  }
  sz_file = ntohl(sz_file);
  printf("Putting file %s size: %d\n", filename, sz_file);

  FILE* file = fopen(filename, "w+b");
  if(!file) {
    fprintf(stderr, "File error\n");
    return -1;
  }

  int run = 1;
  size_t total = 0;
  while(run) {
    char buffer[BUF_SIZE];
    size_t read_n = read(sock, buffer, 255);
    total += read_n;

    if(total == sz_file)
      run = 0;

    if(read_n != fwrite(buffer, 1, read_n, file)) {
      fprintf(stderr, "File writing error\n");
      fclose(file);
      return -1;
    }
  }
  fclose(file);
  return 0;
}

char* prefix_it(const char* in, const char* prefix) {
  char* out;
  size_t len1, len2;
  len1 = strlen(prefix); len2 = strlen(in);
  out = malloc(len1 + len2  + 1);
  if(out == NULL) {
    fprintf(stderr, "PANIC!!!!!");
    exit(EXIT_FAILURE);
  }

  memcpy(out, prefix, len1);
  strcpy(out + len1, in);
  return out;
}
