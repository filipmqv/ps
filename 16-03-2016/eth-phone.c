/*
 *
 * Compilation:  gcc -Wall ./ethrecv.c -o ./ethrecv
 * Usage:        ./eth-phone IF(interfejs nadawcy i odbiorcy - ten sam)  DST_HW_ADDR(mac odbiorcy)
 * NOTE:         This program requires root privileges.
 *
 *
 */

#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ETH_P_CUSTOM 0x8888

char* fdataToSend;

int ethsend(char* argv1, char* argv2) {
  int sfd, ifindex;
  char* frame;
  char* fdata;
  struct ethhdr* fhead;
  struct ifreq ifr;
  struct sockaddr_ll sall;

  sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_CUSTOM));
  frame = malloc(ETH_FRAME_LEN);
  memset(frame, 0, ETH_FRAME_LEN);
  fhead = (struct ethhdr*) frame;
  fdata = frame + ETH_HLEN;
  strncpy(ifr.ifr_name, argv1, IFNAMSIZ);
  ioctl(sfd, SIOCGIFINDEX, &ifr);
  ifindex = ifr.ifr_ifindex;
  ioctl(sfd, SIOCGIFHWADDR, &ifr);
  memset(&sall, 0, sizeof(struct sockaddr_ll));
  sall.sll_family = AF_PACKET;
  sall.sll_protocol = htons(ETH_P_CUSTOM);
  sall.sll_ifindex = ifindex;
  sall.sll_hatype = ARPHRD_ETHER;
  sall.sll_pkttype = PACKET_OUTGOING;
  sall.sll_halen = ETH_ALEN;
  sscanf(argv2, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &sall.sll_addr[0], &sall.sll_addr[1], &sall.sll_addr[2],
         &sall.sll_addr[3], &sall.sll_addr[4], &sall.sll_addr[5]);
  memcpy(fhead->h_dest, &sall.sll_addr, ETH_ALEN);
  memcpy(fhead->h_source, &ifr.ifr_hwaddr.sa_data, ETH_ALEN);
  fhead->h_proto = htons(ETH_P_CUSTOM);
  memcpy(fdata, fdataToSend, strlen(fdataToSend) + 1);
  sendto(sfd, frame, ETH_HLEN + strlen(fdataToSend) + 1, 0,
         (struct sockaddr*)&sall, sizeof(struct sockaddr_ll));
  printf("sent\n");
  free(frame);
  close(sfd);
  return EXIT_SUCCESS;
}



int main(int argc, char** argv) {
  int sfd, i;
  ssize_t len;
  char* frame;
  
  struct ethhdr* fhead;
  struct ifreq ifr;
  struct sockaddr_ll sall;

  sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_CUSTOM));
  strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);
  ioctl(sfd, SIOCGIFINDEX, &ifr);
  memset(&sall, 0, sizeof(struct sockaddr_ll));
  sall.sll_family = AF_PACKET;
  sall.sll_protocol = htons(ETH_P_CUSTOM);
  sall.sll_ifindex = ifr.ifr_ifindex;
  sall.sll_hatype = ARPHRD_ETHER;
  sall.sll_pkttype = PACKET_HOST;
  sall.sll_halen = ETH_ALEN;
  bind(sfd, (struct sockaddr*) &sall, sizeof(struct sockaddr_ll));
  while(1) {
    frame = malloc(ETH_FRAME_LEN);
    memset(frame, 0, ETH_FRAME_LEN);
    fhead = (struct ethhdr*) frame;
    fdataToSend = frame + ETH_HLEN;
    len = recvfrom(sfd, frame, ETH_FRAME_LEN, 0, NULL, NULL);
    printf("[%dB] %02x:%02x:%02x:%02x:%02x:%02x -> ", (int)len,
           fhead->h_source[0], fhead->h_source[1], fhead->h_source[2],
           fhead->h_source[3], fhead->h_source[4], fhead->h_source[5]);
    printf("%02x:%02x:%02x:%02x:%02x:%02x | ",
           fhead->h_dest[0], fhead->h_dest[1], fhead->h_dest[2],
           fhead->h_dest[3], fhead->h_dest[4], fhead->h_dest[5]);
    printf("%s\n", fdataToSend);
    for (i = 0; i < len ; i++) {
      printf("%02x ", (unsigned char) frame[i]);
      if ((i + 1) % 16 == 0)
        printf("\n");
    }
    printf("\n\n");
    
    ethsend(argv[1], argv[2]);
    
    free(frame);
  }
  close(sfd);
  return EXIT_SUCCESS;
}
