/*
 *
 * Compilation:  gcc -Wall ./ipping.c -o ./ipping
 * Usage:        ./ipping DST_IP_ADDR
 * NOTE:         This program requires root privileges.
 *
 *
 */

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

int tdiff(struct timeval t1, struct timeval t2) {
  return (t2.tv_sec - t1.tv_sec) * 1000000
         + ((int)t2.tv_usec - (int)t1.tv_usec);
}

uint16_t chksum(uint16_t *addr, int len) {
  int nleft = len, sum = 0;
  uint16_t *w = addr, u = 0, result;

  while(nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }
  if (nleft == 1) {
    *(u_char*) &u = *(u_char*) w;
    sum += u;
  }
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  result = ~sum;
  return result;
}

int main(int argc, char **argv) {
  int sfd, us;
  socklen_t sl;
  char buf[2048];
  struct timeval t1, t2;
  struct sockaddr_in snd, rcv;
  struct icmphdr req;

  sfd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
  memset(&snd, 0, sizeof(snd));
  snd.sin_family = AF_INET;
  snd.sin_port = 0;
  snd.sin_addr.s_addr = inet_addr(argv[1]);
  memset(&req, 0, sizeof(req));
  req.type = ICMP_ECHO;
  req.code = 0;
  req.un.echo.id = 1234;
  req.un.echo.sequence = 0;
  while(1) {
    printf("REQ %s\n", argv[1]);
    req.un.echo.sequence += 1;
    req.checksum = 0;
    req.checksum = chksum((uint16_t*) &req, sizeof(req));
    gettimeofday(&t1, NULL);
    sendto(sfd, &req, sizeof(req), 0, (struct sockaddr*) &snd, sizeof(snd));
    sl = sizeof(rcv);
    recvfrom(sfd, &buf, sizeof(buf), 0, (struct sockaddr*) &rcv, &sl);
    gettimeofday(&t2, NULL);
    if (rcv.sin_addr.s_addr == snd.sin_addr.s_addr) {
      // check if type is ICMP_ECHOREPLY
      struct iphdr *ip;
      struct icmphdr *data;
      ip = (struct iphdr*) &buf;
      data = (struct icmphdr*) ip + (ip->ihl * 4);
      if (data->type == ICMP_ECHOREPLY) {
        us = tdiff(t1, t2);
        printf("REP %s (%d.%04d s)\n", argv[1], us / 1000000, us % 1000000);
      } else {
        printf("REP is not type of ICMP_ECHOREPLY");
        continue;
      }
    }
    sleep(1);
  }
  close(sfd);
  return EXIT_SUCCESS;
}
