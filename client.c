#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  if(argc != 4) {
    //wrong argc
    printf("Wrong number of arguments \n");
    exit(EXIT_FAILURE);
  }
  const char* op = argv[1];
  const char* file = argv[2];
  const char* server = argv[3];

  if(strcmp(op, "put")) {
    printf("Putting file: %s to %s \n", file, server);
  } else if(strcmp(op, "get")) {
    printf("Getting file: %s from %s \n", file, server);
  }

  exit(EXIT_SUCCESS);
}
