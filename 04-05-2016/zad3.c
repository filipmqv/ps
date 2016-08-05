/*
 *
 * Compilation:  gcc -Wall ./zad3.c -o ./zad3
 * Usage:        ./zad3 DST_IP_ADDR
 * NOTE:         This program requires root privileges.
 *
 *
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define IPPROTO_CUSTOM 222

int main(int argc, char **argv) {
  int sfd, rc;
  char buf[65536], saddr[16], daddr[16];
  char *data;
  socklen_t sl;
  struct sockaddr_in addr;
  struct iphdr *ip;

  sfd = socket(PF_INET, SOCK_RAW, IPPROTO_CUSTOM);
  while(1) {
    // receive:
    memset(&addr, 0, sizeof(addr));
    sl = sizeof(addr);
    rc = recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr*) &addr, &sl);
    ip = (struct iphdr*) &buf;
    if (ip->protocol == IPPROTO_CUSTOM) {
      inet_ntop(AF_INET, &ip->saddr, (char*) &saddr, 16);
      inet_ntop(AF_INET, &ip->daddr, (char*) &daddr, 16);
      data = (char*) ip + (ip->ihl * 4);
      printf("[%dB]\t %s -> %s | %s\n", rc - (ip->ihl * 4), saddr, daddr, data);
      
      // send:
      struct sockaddr_in sendaddr;
      memset(&sendaddr, 0, sizeof(sendaddr));
      sendaddr.sin_family = AF_INET;
      sendaddr.sin_port = 0;
      sendaddr.sin_addr.s_addr = inet_addr(argv[1]);
      sendto(sfd, data, strlen(data) + 1, 0, (struct sockaddr*) &sendaddr, sizeof(sendaddr));
      printf("forw:\t %s -> %s | %s\n", daddr, argv[1], data);
      
    }
  }
  close(sfd);
  return EXIT_SUCCESS;
}
