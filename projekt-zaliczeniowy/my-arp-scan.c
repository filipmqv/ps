/*
 * Compilation:  gcc my-arp-scan.c -o my-arp-scan -lnet -lpcap -lm    (-lm for math library)
 * Usage:        ./my-arp-scan INTERFACE(e.g. em1)
 * NOTE:         This program requires root privileges.
 */

#include <libnet.h>
#include <stdlib.h>
#include <pcap.h>
#include <signal.h>
#include <string.h>
#include <linux/if_ether.h>
#include <netinet/ip.h>
#include <time.h>
#include <math.h>

#define IPPROTO_CUSTOM 222
#define SEARCH_TIMEOUT 20.0f
#define FOUND_SIZE 1024
#define IP_ARRAY_LEN 32

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

char* errbuf2;
pcap_t* handle;
libnet_t *ln;
u_int32_t target_ip_addr, src_ip_addr;
bpf_u_int32 netp, maskp;

// raw ip packet
int sfd;

// arrays to collect data, buff to respond to initiator
char* found_ips[FOUND_SIZE];
char* found_macs[FOUND_SIZE];
int num_of_found;
char response_buf[1500];
char error_to_send[1500];

void cleanup() {
    libnet_destroy(ln);
    pcap_close(handle);
    free(errbuf2);
    close(sfd);
}

void stop(int signo) {
    exit(EXIT_SUCCESS);
}

void int_to_binary(long int input, int* result) {
    int i;
    for (i = 0; i < IP_ARRAY_LEN; i++)
        result[i] = 0;
    i = 0;
    while (input != 0) {
         result[i++]= input % 2;
         input = input / 2;
    }
}

int count_zeros(int* input) {
    int num = 0, i;
    for (i = 0; i < IP_ARRAY_LEN; i++) {
        if (input[i] == 0)   
            num++;
    }
    return num;
}

int num_of_devices_from_zeros(int input) {
    return (int)(pow(2, input) - 2);
}

int num_of_devices_from_mask(long int input_mask) {
    int bina[IP_ARRAY_LEN];
    int_to_binary(input_mask, bina);
    int num_of_zeros = count_zeros(bina);
    return num_of_devices_from_zeros(num_of_zeros);
}

// -1 - nope, 0+ - return index
int was_ip_found_before(char* searched_addr) {
    int i;
    for (i = 0; i < num_of_found; i++) {
        if (strcmp(found_ips[i], searched_addr) == 0) {
            return i;
        }
    }
    return -1;
}

long int get_next_ip_to_send_arp(unsigned char* b) {
    b[3]++;
    int i;
    // check for overflow
    for (i = 3; i > 0; i--) {
        if (b[i] == 255) {
            b[i] = 1; 
            b[i-1]++;
        }
    }
    // e.g.    64.233.187.99
    // 99*2^24 + 187*2^16 + 233*2^8 + 64*2^0
    return b[3]*16777216 + b[2]*65536 + b[1]*256 + b[0];
}

// returns 0 if ok, -1 if error
int send_arp_requests() {
    u_int8_t bcast_hw_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    zero_hw_addr[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    struct libnet_ether_addr* src_hw_addr;

    src_ip_addr = libnet_get_ipaddr4(ln);
    src_hw_addr = libnet_get_hwaddr(ln);

    int num_of_devices = num_of_devices_from_mask(maskp);
    
    /*long int mymasktest;  
    inet_pton(AF_INET, "255.255.248.0", &mymasktest);
    printf("mymasktest %ld\n", mymasktest);
    
    long int myiptest;
    inet_pton(AF_INET, "150.254.254.32", &myiptest); 
    printf("myiptest %ld\n", myiptest);*/

    unsigned char ip_bytes[4];
    ip_bytes[0] = netp & 0xFF;
    ip_bytes[1] = (netp >> 8) & 0xFF;
    ip_bytes[2] = (netp >> 16) & 0xFF;
    ip_bytes[3] = (netp >> 24) & 0xFF;
    //printf("from bytes %d.%d.%d.%d\n", ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);

    int i;
    for (i = 0; i < num_of_devices; i++) {
        target_ip_addr = get_next_ip_to_send_arp(ip_bytes);
        if (libnet_autobuild_arp(
                    ARPOP_REQUEST,                   /* operation type       */
                    src_hw_addr->ether_addr_octet,   /* sender hardware addr */
                    (u_int8_t*) &src_ip_addr,        /* sender protocol addr */
                    zero_hw_addr,                    /* target hardware addr */
                    (u_int8_t*) &target_ip_addr,     /* target protocol addr */
                    ln)                              /* libnet context       */
            == -1) {
            printf("error libnet_autobuild_arp ## %s\n", libnet_geterror(ln));
            sprintf(error_to_send, "error libnet_autobuild_arp ## %s\n", libnet_geterror(ln));
            return -1;
        }
        if (libnet_autobuild_ethernet(
                    bcast_hw_addr,                   /* ethernet destination */
                    ETHERTYPE_ARP,                   /* ethertype            */
                    ln)                              /* libnet context       */
            == -1) {
            printf("error libnet_autobuild_ethernet ## %s\n", libnet_geterror(ln));
            sprintf(error_to_send, "error libnet_autobuild_ethernet ## %s\n", libnet_geterror(ln));
            return -1;
        }

        if (libnet_write(ln) == -1) {
            printf("libnet_write_error ## %s\n", libnet_geterror(ln));
            sprintf(error_to_send, "libnet_write_error ## %s\n", libnet_geterror(ln));
            return -1;
        }
        libnet_clear_packet(ln);

        // for printing
        //char given_ip[30];
        //inet_ntop(AF_INET, &target_ip_addr, (char*) &given_ip, 16);
        //printf("REQ to %s %ld sent\n", given_ip, target_ip_addr);
    }
    return 0;
}

void trap(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes) {
    struct arphdr* aheader;
    aheader = (struct arphdr*) (bytes + ETH_HLEN);

    if(ntohs(aheader->opcode) == ARPOP_REPLY) {
        char trap_buf[30];
        sprintf(trap_buf, "%d.%d.%d.%d", aheader->sender_ip_addr[0], 
            aheader->sender_ip_addr[1], aheader->sender_ip_addr[2], aheader->sender_ip_addr[3]);
        if (was_ip_found_before(trap_buf) == -1) { // do if wasn't found
            strcpy(found_ips[num_of_found], trap_buf);
            unsigned char* mac = (unsigned char *) aheader->sender_mac_addr;
            sprintf(trap_buf, "%02x:%02x:%02x:%02x:%02x:%02x",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            strcpy(found_macs[num_of_found], trap_buf);
            printf("found: %s \t%s\n", found_ips[num_of_found], found_macs[num_of_found]);
            num_of_found++;
        }
    }
}

void check_arp_responses() {
    int renew[(int) SEARCH_TIMEOUT - 1] = {0};
    int last_second = 1;

    time_t start = time(NULL);
    while((double)(time(NULL) - start) < SEARCH_TIMEOUT) {
        pcap_dispatch(handle, -1, trap, NULL);
        usleep(100000); // 100 ms
        if ((double)(time(NULL) - start) > last_second && renew[last_second] == 0) {
            renew[last_second++] = 1;
            send_arp_requests();  // resend every second
        }
    }
}

void test_machines_with_arp() {
    // init
    strcpy(response_buf, "");
    num_of_found = 0;
    int i;
    for (i = 0; i < FOUND_SIZE; i++) {
        found_ips[i] = (char *) malloc(30 * sizeof(char));
        found_macs[i] = (char *) malloc(30 * sizeof(char));
    }
    // if all requests sent correctly wait for responses and create final response string
    if (send_arp_requests() == 0) {
        check_arp_responses();
        for (i = 0; i < num_of_found; i++) {
            strcat(response_buf, found_ips[i]);
            strcat(response_buf, "\t");
            strcat(response_buf, found_macs[i]);
            strcat(response_buf, "\n");
        }
    } else {
        // first sending returned error
        strcpy(response_buf, error_to_send);
    }
    // free arrays 
    for (i = 0; i < FOUND_SIZE; i++) {
        free(found_ips[i]);
        free(found_macs[i]);
    }
}

int main(int argc, char** argv) {
    atexit(cleanup);
    signal(SIGINT, stop);
    
    char errbuf[LIBNET_ERRBUF_SIZE];
    ln = libnet_init(LIBNET_LINK, argv[1], errbuf);
    
    handle = pcap_create(libnet_getdevice(ln), errbuf2);
    pcap_activate(handle);
    pcap_setnonblock(handle, 1, errbuf2);
    pcap_lookupnet(argv[1], &netp, &maskp, errbuf2);

    int rc;
    char buf[65536], saddr[16], daddr[16];
    char *data;
    socklen_t sl;
    struct sockaddr_in addr;
    struct iphdr *ip;

    sfd = socket(PF_INET, SOCK_RAW, IPPROTO_CUSTOM);
    while(1) {
        memset(&addr, 0, sizeof(addr));
        sl = sizeof(addr);
        // wait for signal from initiator to begin test
        rc = recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr*) &addr, &sl);
        ip = (struct iphdr*) &buf;
        data = (char*) ip + (ip->ihl * 4);
        if (ip->protocol == IPPROTO_CUSTOM && strcmp(data, "check") == 0) {
            inet_ntop(AF_INET, &ip->saddr, (char*) &saddr, 16);
            inet_ntop(AF_INET, &ip->daddr, (char*) &daddr, 16);
            printf("[%dB] %s -> %s | %s\n", rc - (ip->ihl * 4), saddr, daddr, data);

            printf("test started\n");
            test_machines_with_arp();
            printf("test finished\n");

            // send response:
            struct sockaddr_in sendaddr;
            memset(&sendaddr, 0, sizeof(sendaddr));
            sendaddr.sin_family = AF_INET;
            sendaddr.sin_port = 0;
            sendaddr.sin_addr.s_addr = inet_addr(saddr);
            if (sendto(sfd, response_buf, strlen(response_buf) + 1, 0, 
                    (struct sockaddr*) &sendaddr, sizeof(sendaddr)) == -1) {
                printf("error sending response\n");
            } else {
                printf("response sent to:\t %s | message: \n ### \n%s ### \n", saddr, response_buf);
            }
        }
    }
    close(sfd);    
    return EXIT_SUCCESS;
}
