/*
 *
 * Compilation:  gcc -Wall ./ipv6-tcp-server.c -o ./ipv6-tcp-server
 * Usage:        ./ipv6-tcp-server
 *
 *
 */

#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
  socklen_t sl;
  int sfd, cfd, on = 1;
  struct sockaddr_in6 saddr, caddr;

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin6_family = AF_INET6;
  saddr.sin6_addr = in6addr_any;
  saddr.sin6_port = htons(1234);
  sfd = socket(PF_INET6, SOCK_STREAM, 0);
  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));
  bind(sfd, (struct sockaddr*) &saddr, sizeof(saddr));
  listen(sfd, 5);
  while(1) {
    memset(&caddr, 0, sizeof(caddr));
    sl = sizeof(caddr);
    cfd = accept(sfd, (struct sockaddr*) &caddr, &sl);
    write(cfd, "Hello World!\n", 14);
    close(cfd);
  }
  close(sfd);
  return EXIT_SUCCESS;
}
