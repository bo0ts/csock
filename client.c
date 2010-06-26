#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "common.h"

#define PORT "6969"

static const char* prefix = "./downloading/";

//free it yourself
static char* prefix_it(const char* in) {
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


//returns a socket connected to the server
int connect_serv(const char* server) {
  struct addrinfo hints;
  struct addrinfo* result;
  struct addrinfo* rp;
  int sfd, s;

  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = PF_INET;          /* Any protocol */

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
    sfd = -1;
  }

  if (rp == NULL) {               /* No address succeeded */
    fprintf(stderr, "Could not bind\n");
    exit(EXIT_FAILURE);
  }
  
  freeaddrinfo(result);
  
  return sfd;
}

int put(const char* file, const char* server) {
  int sfd = connect_serv(server);
  if(sfd < 0) {
    fprintf(stderr, "Error connecting");
    exit(EXIT_FAILURE);
  }
    
  //create headers and send them
  msg_header_t msg_header;
  data_header_t data_header;
  
  make_msg_header(&msg_header, REQUEST_UP, file);
  make_data_header(&data_header, file);
  
  if(msg_header.size != send(sfd, msg_header.data, msg_header.size, 0)) {
    fprintf(stderr, "Socket write with msg_header failed\n");
    exit(EXIT_FAILURE);
  }

  if(data_header.size != write(sfd, data_header.data, data_header.size)) {
    fprintf(stderr, "Socket write with data_header failed\n");
    exit(EXIT_FAILURE);
  }

  free_data_header(&data_header);

  printf("Checking for reply\n");

  //we expect the reply to be the same as our msg_header_data just with another opcode
  //we change change the opcode of our msg_header to the expected and just memcmp
  //the data
  change_opcode(&msg_header, REPLY_UP);

  char* rep_buffer = calloc(msg_header.size, 1);
  
  if(msg_header.size != read(sfd, rep_buffer, msg_header.size)) {
    fprintf(stderr, "Socket read failed\n");
    exit(EXIT_FAILURE);
  }
  
  if(0 != memcmp(rep_buffer, msg_header.data, msg_header.size)) {
    fprintf(stderr, "Reply was bogus, go panic\n");
  } else {
    printf("Success!\n");
  }

  free_msg_header(&msg_header);
  free(rep_buffer);
  close(sfd);

  return EXIT_SUCCESS;
}

int get(const char* file, const char* server) {
  int sfd = connect_serv(server);
  if(sfd < 0) {
    fprintf(stderr, "Error connecting");
    exit(EXIT_FAILURE);
  }

  msg_header_t msg_header;
  make_msg_header(&msg_header, REQUEST_DOWN, file);
  if(msg_header.size != send(sfd, msg_header.data, msg_header.size, 0)) {
    fprintf(stderr, "Socket write with msg_header failed\n");
    exit(EXIT_FAILURE);
  }
  change_opcode(&msg_header, REPLY_DOWN);

  char* rep_buffer = calloc(msg_header.size, 1);
  
  if(msg_header.size != read(sfd, rep_buffer, msg_header.size)) {
    fprintf(stderr, "Socket read failed\n");
    exit(EXIT_FAILURE);
  }
  
  if(msg_header.size != read(sfd, rep_buffer, msg_header.size)) {
    fprintf(stderr, "Socket read failed\n");
    exit(EXIT_FAILURE);
  }
  
  if(0 != memcmp(rep_buffer, msg_header.data, msg_header.size)) {
    fprintf(stderr, "Reply was bogus, go panic\n");
    exit(EXIT_FAILURE);
  }

  int file;
  char buffer[4];
  if(4 != recv(fd, buffer, 4, 0)) {
    fprintf(stderr, "read failed\n");
    return 0;
  }

  uint32_t file_sz = 0;
  memcpy(&file_sz, buffer, 4);
  file_sz = ntohl(file_sz);

  char* file_buffer;
  file_buffer = calloc(file_sz, 1);
  if(file_buffer == NULL) {
    fprintf(stderr, "PANIC!!!!!");
    free(file_buffer);
    exit(EXIT_FAILURE);
  }

  if(file_sz != recv(fd, file_buffer, file_sz, 0)) {
    fprintf(stderr, "read failed\n");
    free(file_buffer);
    return 0;
  }

  char* fn_altered = prefix_it(filename);
  file = open(fn_altered, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if(file_sz != write(file, file_buffer, file_sz)) {
    //bork and send error
    fprintf(stderr, "File write failed\n");
    free(file_buffer);
  }

  free(file_buffer);
  free(fn_altered);
  free_msg_header(&msg_header);
  free(rep_buffer);
  close(sfd);

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
