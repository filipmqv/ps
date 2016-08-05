/*
 *
 * Compilation:  gcc -Wall ./setgw.c -o ./setgw
 * Usage:        ./setgw GWIP(gateway ip)
 * NOTE:         This program requires root privileges.
 *
 *
 */

#include <arpa/inet.h>
#include <linux/route.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char** argv) {
  int fd;
  struct rtentry route;
  struct sockaddr_in* addr;

  fd = socket(PF_INET, SOCK_DGRAM, 0);
  memset(&route, 0, sizeof(route));
  addr = (struct sockaddr_in*) &route.rt_gateway;
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = inet_addr(argv[1]);
  addr = (struct sockaddr_in*) &route.rt_dst;
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = INADDR_ANY;
  addr = (struct sockaddr_in*) &route.rt_genmask;
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = INADDR_ANY;
  route.rt_flags = RTF_UP | RTF_GATEWAY;
  route.rt_metric = 0;
  ioctl(fd, SIOCADDRT, &route);
  close(fd);
  return EXIT_SUCCESS;
}
