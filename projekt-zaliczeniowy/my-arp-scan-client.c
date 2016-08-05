/*
* Compilation:  gcc -Wall my-arp-scan-client.c -o my-arp-scan-client
* Usage:        ./my-arp-scan-client NUM_OF_SERVERS [DST_IP_ADDRESSES...]
* NOTE:         This program requires root privileges.
*/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define IPPROTO_CUSTOM 222

int sfd;

char* servers[1024];
int responses[1024]; // 1 - got response, 0 - not yet
int num_of_servers;

void cleanup() {
    close(sfd);
}

void stop(int signo) {
    exit(EXIT_SUCCESS);
}

// -1 - not found, 0+ - return index
int index_of_server(char* addr) {
    int i;
    for (i = 0; i < num_of_servers; i++) 
        if (strcmp(servers[i], addr) == 0) 
            return i;
    return -1;
}

// 0 - true, -1 - false
int are_all_responses() {
    int i;
    for (i = 0; i < num_of_servers; i++) {
        if (responses[i] == 0)
            return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    atexit(cleanup);
    signal(SIGINT, stop);

    int rc;
    char buf[65536], saddr[16], daddr[16];
    char *data;
    socklen_t sl;
    struct sockaddr_in addr;
    struct iphdr *ip;

    sscanf(argv[1], "%d", &num_of_servers);

    int i;
    for (i = 0; i < num_of_servers; i++) {
        servers[i] = argv[2+i];
        responses[i] = 0;
    }

    sfd = socket(PF_INET, SOCK_RAW, IPPROTO_CUSTOM);

    // send requests:
    struct sockaddr_in sendaddr;
    memset(&sendaddr, 0, sizeof(sendaddr));
    sendaddr.sin_family = AF_INET;
    sendaddr.sin_port = 0;
    data = "check";
    for (i = 0; i < num_of_servers; i++) {
        sendaddr.sin_addr.s_addr = inet_addr(servers[i]);
        sendto(sfd, data, strlen(data) + 1, 0, (struct sockaddr*) &sendaddr, sizeof(sendaddr));
        printf("request sent to: %s | %s\n", servers[i], data);
    }
    
    
    // receive responses:
    while(1) {
        memset(&addr, 0, sizeof(addr));
        sl = sizeof(addr);
        rc = recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr*) &addr, &sl);
        ip = (struct iphdr*) &buf;
        data = (char*) ip + (ip->ihl * 4);
        if (ip->protocol == IPPROTO_CUSTOM && strcmp(data, "check") != 0) {
            inet_ntop(AF_INET, &ip->saddr, (char*) &saddr, 16);
            inet_ntop(AF_INET, &ip->daddr, (char*) &daddr, 16);
            
            printf("[%dB] response from %s | message:\n ### \n%s ### \n", 
                    rc - (ip->ihl * 4), saddr, data);
            int index = index_of_server(saddr);
            if (index >= 0)
                responses[index] = 1;

            if (are_all_responses() == 0) {
                printf("all servers responded\n");
                break;
            }
        }
    }

    close(sfd);
    return EXIT_SUCCESS;
}