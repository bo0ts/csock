#ifndef _COMMON_H__
#define _COMMON_H__

#include <inttypes.h>
#include <unistd.h>

#define REQUEST_DOWN 0x01
#define REPLY_DOWN 0x02
#define REQUEST_UP 0x03
#define REPLY_UP 0x04
#define ERROR 0x05

const char* op_tbl[6];

typedef struct {
  char* data;
  ssize_t size;
} msg_header_t;

typedef struct {
  char* data;
  ssize_t size;
} data_header_t;

//given opcode and filename create a msg header
void make_msg_header(msg_header_t* header, uint16_t op, const char* filename);
void change_opcode(msg_header_t* header, uint16_t op);
void free_msg_header(msg_header_t* header);

//given a filename create a data "header"
void make_data_header(data_header_t* header, const char* filename);
void free_data_header(data_header_t* header);

//file socket interaction
int file_to_socket(int sock, const char* filename);
int socket_to_file(int sock, const char* filename);

char* prefix_it(const char* in, const char* prefix);

#endif //header guard
