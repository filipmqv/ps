/*
 *
 * Compilation:  gcc zad4.c -o zad4
 * Usage:        ./zad4 IF(interface to listen on)
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

#include <linux/route.h>
#include <netinet/in.h>


#define ETH_P_CUSTOM 0x8888

#define IRI_T_ADDRESS 0
#define IRI_T_ROUTE 1
struct ifrtinfo {
    int iri_type;                   /*msg type*/
    char iri_iname[16];             /*ifname*/
    struct sockaddr_in iri_iaddr;   /*IP address*/
    struct sockaddr_in iri_rtdst;   /*dst. IP address*/
    struct sockaddr_in iri_rtmsk;   /*dst. netmask*/
    struct sockaddr_in iri_rtgip;   /*gateway IP*/
};

void ifsetup(char* IFNAME, struct sockaddr_in IP) {
  int fd;
  struct ifreq ifr;
  struct sockaddr_in* sin;

  fd = socket(PF_INET, SOCK_DGRAM, 0);
  strncpy(ifr.ifr_name, IFNAME, strlen(IFNAME) + 1);
  sin = (struct sockaddr_in*) &ifr.ifr_addr;
  memset(sin, 0, sizeof(struct sockaddr_in));
  sin->sin_family = AF_INET;
  sin->sin_port = 0;
  sin->sin_addr = IP.sin_addr;
  ioctl(fd, SIOCSIFADDR, &ifr);
  ioctl(fd, SIOCGIFFLAGS, &ifr);
  ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
  ioctl(fd, SIOCSIFFLAGS, &ifr);
  close(fd);
}


void setgw(struct sockaddr_in DSTIP, struct sockaddr_in MASK, struct sockaddr_in GWIP) {
  int fd;
  struct rtentry route;
  struct sockaddr_in* addr;

  fd = socket(PF_INET, SOCK_DGRAM, 0);
  memset(&route, 0, sizeof(route));
  addr = (struct sockaddr_in*) &route.rt_gateway;
  addr->sin_family = AF_INET;
  addr->sin_addr = GWIP.sin_addr;
  addr = (struct sockaddr_in*) &route.rt_dst;
  addr->sin_family = AF_INET;
  addr->sin_addr = DSTIP.sin_addr;
  addr = (struct sockaddr_in*) &route.rt_genmask;
  addr->sin_family = AF_INET;
  addr->sin_addr = MASK.sin_addr;
  route.rt_flags = RTF_UP | RTF_GATEWAY;
  route.rt_metric = 0;
  ioctl(fd, SIOCADDRT, &route);
  close(fd);
}


int main(int argc, char** argv) {
    int sfd, i;
    ssize_t len;
    char* frame;
    char* fdata;
    struct ifrtinfo* fdatastruct;
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
        fdata = frame + ETH_HLEN;
        len = recvfrom(sfd, frame, ETH_FRAME_LEN, 0, NULL, NULL);
        printf("[%dB] %02x:%02x:%02x:%02x:%02x:%02x -> ", (int)len,
               fhead->h_source[0], fhead->h_source[1], fhead->h_source[2],
               fhead->h_source[3], fhead->h_source[4], fhead->h_source[5]);
        printf("%02x:%02x:%02x:%02x:%02x:%02x | ",
               fhead->h_dest[0], fhead->h_dest[1], fhead->h_dest[2],
               fhead->h_dest[3], fhead->h_dest[4], fhead->h_dest[5]);
        printf("%s\n", fdata);
        for (i = 0; i < len ; i++) {
            printf("%02x ", (unsigned char) frame[i]);
            if ((i + 1) % 16 == 0)
                printf("\n");
        }
        printf("\n\n");
        
        fdatastruct = (struct ifrtinfo*) fdata;
        switch( fdatastruct->iri_type )
        {
            case IRI_T_ADDRESS:
                printf("set IRI_T_ADDRESS\n");
                ifsetup((char*) &fdatastruct->iri_iname, fdatastruct->iri_iaddr);
                break;
                
            case IRI_T_ROUTE:
                printf("set IRI_T_ROUTE\n");
                setgw(fdatastruct->iri_rtdst, fdatastruct->iri_rtmsk, fdatastruct->iri_rtgip);
                break;
                
            default:
                printf("error\n");
                break;
        }
        
        free(frame);
    }
    close(sfd);
    return EXIT_SUCCESS;
}
