#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "common.h"

#define PORT "6969"

int put(const char* file, const char* server) {
  struct addrinfo hints;
  struct addrinfo* result;
  struct addrinfo* rp;
  int sfd, s;
  FILE* fd;


  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;          /* Any protocol */

  s = getaddrinfo(server, PORT, NULL, &result);

  if(s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    freeaddrinfo(result);
    exit(EXIT_FAILURE);
  }
  
  for (rp = result; rp != NULL; rp = rp->ai_next) {  
    sfd = socket(rp->ai_family, rp->ai_socktype,
		 rp->ai_protocol);
    if (sfd == -1)
      continue;

    if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
      break;                  /* Success */

    close(sfd);
  }

  if (rp == NULL) {               /* No address succeeded */
    fprintf(stderr, "Could not bind\n");
    exit(EXIT_FAILURE);
  }
  
  freeaddrinfo(result);

  header_t header;

  size_t filename_len = strlen(file);
  make_header(&header, REQUEST_UP, filename_len);

  //write the header and filename to the stream
  if(6 != write(sfd, header.data, 6)) {
    fprintf(stderr, "Socket write failed\n");
    exit(EXIT_FAILURE);
  }
  
  //write the filename
  if(filename_len != write(sfd, file, filename_len)) {
    fprintf(stderr, "Socket write failed\n");
    exit(EXIT_FAILURE);
  }
  
  fd = fopen(file, "rb"); //open binary read file

  if(!fd) {
    fprintf(stderr, "File error\n");
    exit(EXIT_FAILURE);
  }

  fseek(fd, 0L, SEEK_END);
  int32_t sz_file = (int32_t)ftell(fd);
  fseek(fd, 0L, SEEK_SET);

  //some status report
  printf("Filename length: %d, Filesize: %d\n", filename_len, sz_file);

  //file length
  if(4 != write(sfd, &sz_file, 4)) {
    fprintf(stderr, "Socket write failed\n");
    exit(EXIT_FAILURE);
  }

  //buffer the file and write it
  void* filebuffer = malloc(sz_file);
  if(filebuffer == NULL) {
    fprintf(stderr, "Out of memory\n");
    exit(EXIT_FAILURE);
  }

  if(sz_file != fread(filebuffer, 1, sz_file, fd)) {
    fprintf(stderr, "File buffer borked\n");
    exit(EXIT_FAILURE);
  }

  if(sz_file != write(sfd, filebuffer, sz_file)) {
    fprintf(stderr, "Socket write failed\n");
    exit(EXIT_FAILURE);
  }

  free(filebuffer);

  //check for reply
  printf("Checking for reply\n");
  size_t reply_size = 6 + filename_len;
  char* rep_buffer = calloc(reply_size, 1); //space for header + filename + zero byte

  if(reply_size != read(sfd, rep_buffer, reply_size)) {
    fprintf(stderr, "Socket read failed\n");
    exit(EXIT_FAILURE);
  }
  
  //does this work?
  if(REPLY_UP != (int16_t)rep_buffer[0]) {
    fprintf(stderr, "Wrong reply\n");
    exit(EXIT_FAILURE);
  }

  if(filename_len != (int32_t)rep_buffer[2]) {
    fprintf(stderr, "Wrong filename length\n");
    exit(EXIT_FAILURE);
  }

  if(!memcmp(&rep_buffer[6], file, filename_len)) {
    fprintf(stderr, "Wrong filename\n");
    exit(EXIT_FAILURE);
  }

  free(rep_buffer);
  close(sfd);

  return EXIT_SUCCESS;
}

int get(const char* file, const char* server) {

  return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
  if(argc != 4) {
    //wrong argc
    printf("Wrong number of arguments \n");
    exit(EXIT_FAILURE);
  }

  const char* op = argv[1];
  const char* file = argv[2];
  const char* server = argv[3];

  if(!strcmp(op, "put")) {
    printf("Putting file: %s to %s \n", file, server);
    return put(file, server);
  } else if(!strcmp(op, "get")) {
    printf("Getting file: %s from %s \n", file, server);
    return get(file, server);
  } else {
    printf("Unknown command \n");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
