#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "common.h"

#define PORT "6969"

static const char* prefix = "./incoming/";

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


int put(int fd, const char* filename) {
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
    return 0;
  }

  if(file_sz != recv(fd, file_buffer, file_sz, 0)) {
    fprintf(stderr, "read failed\n");
    free(file_buffer);
    return 0;
  }

  char* fn_altered = prefix_it(filename);
  file = open(fn_altered, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  //file = open("test.txt", O_WRONLY | O_CREAT);
  if(file_sz != write(file, file_buffer, file_sz)) {
    //bork and send error
    fprintf(stderr, "File write failed\n");
    free(file_buffer);
    free(fn_altered);
    return 0;
  }

  free(file_buffer);
  free(fn_altered);
  return 1;
}

void get(int fd, const char* filename) {

}


int main(void) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sfd, s;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  s = getaddrinfo(NULL, PORT, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  /* getaddrinfo() returns a list of address structures.
     Try each address until we successfully bind(2).
     If socket(2) (or bind(2)) fails, we (close the socket
     and) try the next address. */

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype,
		 rp->ai_protocol);
    if (sfd == -1)
      continue;

    if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
      fprintf(stderr, "Binding!\n");
      break;                /* Success */
    }
    close(sfd);
  }

  if (rp == NULL) {               /* No address succeeded */
    fprintf(stderr, "Could not bind\n");
    exit(EXIT_FAILURE);
  }

  if(!(0 == listen(sfd, 0))) {
    fprintf(stderr, "Listen failed\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(result);           /* No longer needed */

  for (;;) {
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
 
    int sock;

    if(-1 == (sock = accept(sfd, (struct sockaddr *)&their_addr, &addr_size))) {
      fprintf(stderr, "accept failed\n");
      break;
    }

    char header[6];
    if(6 != recv(sock, header, 6, 0)) {
      fprintf(stderr, "read failed\n");
      close(sock); break;
    }

    uint16_t op;
    memcpy(&op, header, 2);
    op = ntohs(op);
    uint32_t fn_sz;
    memcpy(&fn_sz, header + 2, 4);
    fn_sz = ntohl(fn_sz);
    char* fn = malloc(fn_sz + 1);
    
    if(fn_sz != recv(sock, fn, fn_sz, 0)) {
      fprintf(stderr, "read failed\n");
      close(sock); break;
    }
    fn[fn_sz] = '\0';
    
    printf("Retrieved Op %s (%d) with Filename length %d for %s\n", op_tbl[op], op, fn_sz, fn);
    msg_header_t rep;

    if(op == REQUEST_UP) {
      if(put(sock, fn) == 1) {
	make_msg_header(&rep, REPLY_UP, fn);
	if(rep.size != write(sock, rep.data, rep.size)) {
	  fprintf(stderr, "write reply failed\n");
	} 
      } else {
	make_msg_header(&rep, ERROR, fn);
      }
    } else if(op == REQUEST_DOWN) {
      get(sock, fn);
    } else {
      printf("Bogus opcode, rejecting\n");
    }
    
    free_msg_header(&rep);
    close(sock);
  }

  close(sfd);

  return EXIT_SUCCESS;
}
