/*
 *
 * Compilation:  gcc arpingmqv.c -o arpingmqv -lnet -lpcap
 * Usage:        ./arpingmqv HOST
 * NOTE:         This program requires root privileges.
 *
 *
 */

#include <libnet.h>
#include <stdlib.h>
#include <pcap.h>
#include <signal.h>
#include <string.h>
#include <linux/if_ether.h>
#include <netinet/ip.h>

char* errbuf2;
pcap_t* handle;
libnet_t *ln;

void cleanup() {
    libnet_destroy(ln);
    pcap_close(handle);
    free(errbuf2);
}

void stop(int signo) {
    exit(EXIT_SUCCESS);
}

struct arphdr {
    u_int16_t ftype;
    u_int16_t ptype;
    u_int8_t flen;
    u_int8_t plen;
    u_int16_t opcode;
    u_int8_t sender_mac_addr[6];
    u_int8_t sender_ip_addr[4];
    u_int8_t target_mac_addr[6];
    u_int8_t target_ip_addr[4];
};

int main(int argc, char** argv) {
    atexit(cleanup);
    signal(SIGINT, stop);
    u_int32_t target_ip_addr, src_ip_addr;
    u_int8_t bcast_hw_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    zero_hw_addr[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    struct libnet_ether_addr* src_hw_addr;
    char errbuf[LIBNET_ERRBUF_SIZE];
    
    int response;
    const u_char* frame;
    struct pcap_pkthdr* pheader = NULL;
    struct ethhdr* fheader;
    struct arphdr* aheader;
    
    ln = libnet_init(LIBNET_LINK, NULL, errbuf);
    src_ip_addr = libnet_get_ipaddr4(ln);
    src_hw_addr = libnet_get_hwaddr(ln);
    target_ip_addr = libnet_name2addr4(ln, argv[1], LIBNET_RESOLVE);
    libnet_autobuild_arp(
        ARPOP_REQUEST,                   /* operation type       */
        src_hw_addr->ether_addr_octet,   /* sender hardware addr */
        (u_int8_t*) &src_ip_addr,        /* sender protocol addr */
                         zero_hw_addr,                    /* target hardware addr */
                         (u_int8_t*) &target_ip_addr,     /* target protocol addr */
                         ln);                             /* libnet context       */
    libnet_autobuild_ethernet(
        bcast_hw_addr,                   /* ethernet destination */
        ETHERTYPE_ARP,                   /* ethertype            */
        ln);                             /* libnet context       */
    
    
    
    
    handle = pcap_create(libnet_getdevice(ln), errbuf2);
    pcap_activate(handle);
    
    while(1) {
        libnet_write(ln);
        printf("REQ sent\n");
        while(1) {
            response = pcap_next_ex( handle, &pheader, &frame);
            // test response? if != 1 continue
            if (response != 1) 
                continue;
            fheader = (struct ethhdr*) frame;
            
            aheader = (struct arphdr*) (frame + ETH_HLEN);
            
            if(ntohs(aheader->opcode) == ARPOP_REPLY &&
                memcmp(aheader->sender_ip_addr, (u_int8_t*) &target_ip_addr, 4) == 0) {
                printf("RESPONSE\n");
            } else continue;
            break;
        }
        sleep(1);
    }
    
    return EXIT_SUCCESS;
}
