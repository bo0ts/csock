#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT "6969"

int put(const char* file, const char* server) {
  struct addrinfo* result;
  struct addrinfo* rp;
  int sfd;

  if(!getaddrinfo(server, PORT, NULL, &result)) {
    printf("getaddrinfo() bork \n");
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

  

}

int get(const char* file, const char* server) {

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
