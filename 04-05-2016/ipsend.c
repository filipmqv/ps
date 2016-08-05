/*
 *
 * Compilation:  gcc -Wall ./ipsend.c -o ./ipsend
 * Usage:        ./ipsend DST_IP_ADDR DATA
 * NOTE:         This program requires root privileges.
 *
 *
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define IPPROTO_CUSTOM 222

int main(int argc, char **argv) {
  int sfd;
  struct sockaddr_in addr;

  sfd = socket(PF_INET, SOCK_RAW, IPPROTO_CUSTOM);
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  addr.sin_addr.s_addr = inet_addr(argv[1]);
  sendto(sfd, argv[2], strlen(argv[2]) + 1, 0, (struct sockaddr*) &addr,
         sizeof(addr));
  close(sfd);
  return EXIT_SUCCESS;
}
