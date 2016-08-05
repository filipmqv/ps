/*Napisz program, który odbiera wszystkie ramki ze wskazanego jako pierwszy argument wywo-
łania interfejsu sieciowego i wyswietla nastepujace informacje o kazdej ramce: rozmiar, adres
MAC nadawcy, adres ODBIORCY , typ pakietu (sll_pkttype), wartosc EtherType i transmitowane dane.*/



/*
 *
 * Compilation:  gcc -Wall ./ethrecv.c -o ./ethrecv
 * Usage:        ./ethrecv IF(np em1 - E M JEDEN)
 * NOTE:         This program requires root privileges.
 *
 *
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <linux/filter.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int sfd;
struct ifreq ifr;

#define FILTER dns_filter
struct sock_filter dns_filter[] = { // sudo tcpdump -i em1 ip and udp and dst port 53 -dd 
{ 0x28, 0, 0, 0x0000000c },
{ 0x15, 0, 8, 0x00000800 },
{ 0x30, 0, 0, 0x00000017 },
{ 0x15, 0, 6, 0x00000011 },
{ 0x28, 0, 0, 0x00000014 },
{ 0x45, 4, 0, 0x00001fff },
{ 0xb1, 0, 0, 0x0000000e },
{ 0x48, 0, 0, 0x00000010 },
{ 0x15, 0, 1, 0x00000035 },
{ 0x6, 0, 0, 0x0000ffff },
{ 0x6, 0, 0, 0x00000000 }
};

struct sock_fprog bpf = {
  .len = (sizeof(FILTER) / sizeof(FILTER[0])),
  .filter = FILTER
};

void cleanup() {
  ifr.ifr_flags &= ~IFF_PROMISC;
  ioctl(sfd, SIOCSIFFLAGS, &ifr);
  close(sfd);
}

void stop(int signo) { exit(EXIT_SUCCESS); }

int main(int argc, char** argv) {
  int i;
  ssize_t len;
  char* frame;
  char* fdata;
  struct ethhdr* fhead;
  struct sockaddr_ll sall;
  
  atexit(cleanup);
  signal(SIGINT, stop);

  sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  setsockopt(sfd, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof(bpf));
  strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);
  ioctl(sfd, SIOCGIFFLAGS, &ifr);
  ifr.ifr_flags |= IFF_PROMISC;
  ioctl(sfd, SIOCGIFINDEX, &ifr);
  memset(&sall, 0, sizeof(struct sockaddr_ll));
  sall.sll_family = AF_PACKET;
  sall.sll_protocol = htons(ETH_P_ALL);
  sall.sll_ifindex = ifr.ifr_ifindex;
  sall.sll_hatype = ARPHRD_ETHER;
  sall.sll_pkttype = PACKET_OTHERHOST; //PACKET_HOST;
  sall.sll_halen = ETH_ALEN;
  bind(sfd, (struct sockaddr*) &sall, sizeof(struct sockaddr_ll));
  while(1) {
    frame = malloc(ETH_FRAME_LEN);
    memset(frame, 0, ETH_FRAME_LEN);
    fhead = (struct ethhdr*) frame;
    fdata = frame + ETH_HLEN;
    len = recvfrom(sfd, frame, ETH_FRAME_LEN, 0, NULL, NULL);
    printf("[%dB] %02x:%02x:%02x:%02x:%02x:%02x -> ", (int)len,
           fhead->h_source[0], fhead->h_source[1], fhead->h_source[2],
           fhead->h_source[3], fhead->h_source[4], fhead->h_source[5]);
    printf("%02x:%02x:%02x:%02x:%02x:%02x | ",
           fhead->h_dest[0], fhead->h_dest[1], fhead->h_dest[2],
           fhead->h_dest[3], fhead->h_dest[4], fhead->h_dest[5]);
    printf("<data> %s </data>\n", fdata);
    for (i = 0; i < len ; i++) {
      printf("%c", (char) fdata[i]);
    }
    //printf("\n");
    /*for (i = 0; i < len ; i++) {
      printf("%02x ", (unsigned char) frame[i]);
      if ((i + 1) % 16 == 0)
        printf("\n");
    }*/
    printf("\nsll_pkttype: %d\t", sall.sll_pkttype);
    printf("sll_protocol: %d\n", sall.sll_protocol);
    printf("\n\n");
    free(frame);
  }
  close(sfd);
  return EXIT_SUCCESS;
}
