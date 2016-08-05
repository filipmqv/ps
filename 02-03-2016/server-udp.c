#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int sfd; socklen_t sl; char buf[128];
    struct sockaddr_in saddr, caddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(1234);
    sfd = socket(PF_INET, SOCK_DGRAM, 0);
    bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr));
    while(1) {
        memset(&caddr, 0, sizeof(caddr));
        memset(&buf, 0, sizeof(buf));
        sl = sizeof(caddr);
        recvfrom(sfd, buf, 128, 0, (struct sockaddr*)&caddr, &sl);
        printf("otrzymalem: %s\n", buf);
        sendto(sfd, buf, strlen(buf)+1, 0, (struct sockaddr*)&caddr, sl);
    }
        return 0;
}