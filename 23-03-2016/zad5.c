/*
 *
 * Compilation:  gcc -Wall zad5.c -o zad5 -lpcap
 * Usage:        ./zad5 INTERFACE
 * NOTE:         This program requires root privileges.
 *
 *
 * 
 * /usr/include/netinet/ip.h
 */

#include <pcap.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_ether.h>
#include <netinet/ip.h>
 

char* errbuf;
pcap_t* handle;
int allnum, ARPnum, IPnum, IPUDPnum, IPTCPnum, IPOthernum, othernum;

void cleanup() {
  pcap_close(handle);
  free(errbuf);
  printf("\n\n");
  printf("ARP:     \t%d\n", ARPnum);
  printf("IP:      \t%d\n", IPnum);
  printf("\tIP/UDP:  \t%d\n", IPUDPnum);
  printf("\tIP/TCP:  \t%d\n", IPTCPnum);
  printf("\tIP/other:\t%d\n", IPOthernum);
  printf("other:   \t%d\n", othernum);
  printf("all:     \t%d\n", allnum);
}

void stop(int signo) {
  exit(EXIT_SUCCESS);
}

const static char* etype(unsigned int ethertype) {
  static char buf[16];
  switch (ntohs(ethertype) & 0xFFFFU) {
    case 0x0001: return "802_3";
    case 0x0002: return "AX25";
    case 0x0003: return "ALL";
    case 0x0060: return "LOOP";
    case 0x0800: return "IP";
    case 0x0806: return "ARP";
    case 0x8100: return "8021Q";
    case 0x88A8: return "8021AD";
    default:     snprintf(buf, sizeof(buf), "0x%04x",
                          ntohs(ethertype) & 0xFFFFU);
                 return (const char*) buf;
  }
}

void trap(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes) {
  struct ethhdr* ha = (struct ethhdr*)bytes;
  struct iphdr* iph = (struct iphdr*) (bytes + sizeof(struct ethhdr));
  printf("%s\t", etype(ha->h_proto));
  printf("%d\n", iph->protocol);

  allnum++;
  if (etype(ha->h_proto) == "ARP")
    ARPnum++;
  else if (etype(ha->h_proto) == "IP") {
    IPnum++;
    /* IP: 6=TCP, 17=UDP */
    if ((int)iph->protocol == 6)
      IPTCPnum++;
    else if ((int)iph->protocol == 17)
      IPUDPnum++;
    else IPOthernum++;
  } 
  else othernum++;

}



int main(int argc, char** argv) {
  atexit(cleanup);
  signal(SIGINT, stop);
  allnum = 0; ARPnum = 0; IPnum = 0; IPUDPnum = 0; IPTCPnum = 0; IPOthernum = 0; othernum = 0;
  errbuf = malloc(PCAP_ERRBUF_SIZE);
  handle = pcap_create(argv[1], errbuf);
  pcap_set_promisc(handle, 1);
  pcap_set_snaplen(handle, 65535);
  pcap_activate(handle);
  pcap_loop(handle, -1, trap, NULL);
}
