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
    int sfd, cfd;
    socklen_t sl;
    struct sockaddr_in saddr, caddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(1234);
    sfd = socket(PF_INET, SOCK_STREAM, 0);
    bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr));
    listen(sfd, 5);
    while(1) {
        sl = sizeof(caddr);
        cfd = accept(sfd, (struct sockaddr*)&caddr, &sl);
        write(cfd, "Hello World!\n", 13);
        close(cfd);
    }
        return 0;
}