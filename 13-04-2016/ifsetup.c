/*
 *
 * Compilation:  gcc -Wall ./ifsetup.c -o ./ifsetup
 * Usage:        ./ifsetup IFNAME IP
 * NOTE:         This program requires root privileges.
 *
 *
 */

#include <arpa/inet.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char** argv) {
  int fd;
  struct ifreq ifr;
  struct sockaddr_in* sin;

  fd = socket(PF_INET, SOCK_DGRAM, 0);
  strncpy(ifr.ifr_name, argv[1], strlen(argv[1]) + 1);
  sin = (struct sockaddr_in*) &ifr.ifr_addr;
  memset(sin, 0, sizeof(struct sockaddr_in));
  sin->sin_family = AF_INET;
  sin->sin_port = 0;
  sin->sin_addr.s_addr = inet_addr(argv[2]);
  ioctl(fd, SIOCSIFADDR, &ifr);
  ioctl(fd, SIOCGIFFLAGS, &ifr);
  ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
  ioctl(fd, SIOCSIFFLAGS, &ifr);
  close(fd);
  return EXIT_SUCCESS;
}
