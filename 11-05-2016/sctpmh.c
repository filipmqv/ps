/*
 *
 * Compilation:  gcc -Wall ./sctpmh.c -o ./sctpmh -lsctp
 * Usage:        ./sctpmh moj_adres_publiczny 127.0.0.1
 *
 *
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int sfd, port, i, no = argc - 2;
  socklen_t sl = sizeof(struct sockaddr_in);
  struct sockaddr_in addr, *addrs;

  sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP);
  addrs = malloc(sl * no);
  memset(&addr, 0, sl);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(argv[1]);
  addr.sin_port = 0;
  bind(sfd, (struct sockaddr*) &addr, sl);
  getsockname(sfd, (struct sockaddr*) &addr, &sl);
  port = addr.sin_port;
  for(i = 2; i < argc; i++) {
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[i]);
    addr.sin_port = port;
    memcpy(addrs + (i-2), &addr, sl);
  }
  sctp_bindx(sfd, (struct sockaddr*) addrs, no, SCTP_BINDX_ADD_ADDR);
  no = sctp_getladdrs(sfd, 0, (struct sockaddr**) &addrs);
  for (i = 0; i < no; i++) {
    memcpy(&addr, addrs + i, sl);
    printf("%d: %s:%d\n", i, inet_ntoa(addr.sin_addr), addr.sin_port);
  }
  close(sfd);
  return EXIT_SUCCESS;
}
