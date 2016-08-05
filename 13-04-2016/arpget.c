/*
 *
 * Compilation:  gcc -Wall ./arpget.c -o ./arpget
 * Usage:        ./arpget IP
 * NOTE:         This program requires root privileges.
 *
 *
 */

#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IFNAME "em1"

int main (int argc, char** argv) {
  int fd;
  struct arpreq arq;
  struct sockaddr_in* sin;
  unsigned char* mac;

  memset(&arq, 0, sizeof(arq));
  sin = (struct sockaddr_in*) &arq.arp_pa;
  sin->sin_family = AF_INET;
  sin->sin_addr.s_addr = inet_addr(argv[1]);
  strcpy(arq.arp_dev, IFNAME);
  fd = socket(PF_INET, SOCK_DGRAM, 0);
  ioctl(fd, SIOCGARP, &arq);
  printf("%s\t", inet_ntoa(sin->sin_addr));
  if (arq.arp_flags & ATF_COM) {
    mac = (unsigned char *) &arq.arp_ha.sa_data[0];
    printf("%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("\tC");
    if (arq.arp_flags & ATF_PERM) printf("M");
    if (arq.arp_flags & ATF_PUBL) printf("P");
    if (arq.arp_flags & ATF_USETRAILERS) printf("T");
    if (arq.arp_flags & ATF_NETMASK) printf("N");
  } else
    printf("(incomplete)");
  printf("\n");
  close(fd);
  return EXIT_SUCCESS;
}
