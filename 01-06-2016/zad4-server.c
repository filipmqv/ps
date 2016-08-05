/*
 *
 * Compilation:  gcc -Wall ./zad4-server.c -o ./zad4-server
 * Usage:        ./zad4-server
 *
 *
 */

#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int readable(int fd) {
  fd_set exceptionset;
  FD_ZERO(&exceptionset);
  FD_SET(fd, &exceptionset);
  return select(fd + 1, NULL, NULL, &exceptionset, NULL);
}

int main(int argc, char** argv) {
  socklen_t sl;
  int sfd, cfd, on = 1;
  struct sockaddr_in saddr, caddr;

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port = htons(1234);
  sfd = socket(PF_INET, SOCK_STREAM, 0);
  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));
  bind(sfd, (struct sockaddr*) &saddr, sizeof(saddr));
  listen(sfd, 5);
  while(1) {
    memset(&caddr, 0, sizeof(caddr));
    sl = sizeof(caddr);
    cfd = accept(sfd, (struct sockaddr*) &caddr, &sl);
    printf("sadsa");
    if (readable(sfd) > 0)
        printf("is error\n");
    else
        printf("[select] timeout\n");
    //sleep(2);
    //write(cfd, "Hello World!\n", 14);
    close(cfd);
  }
  close(sfd);
  return EXIT_SUCCESS;
}
