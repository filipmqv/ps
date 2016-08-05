#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char buf[128];
char *sendTo;

void sendToNextNode() {
    int sfd2;
    
    struct sockaddr_in saddr2;
    struct hostent* addrent;
    addrent = gethostbyname(sendTo);
    memset(&saddr2, 0, sizeof(saddr2));
    saddr2.sin_family = AF_INET;
    saddr2.sin_port = htons(1234);
    memcpy(&saddr2.sin_addr.s_addr, addrent->h_addr, addrent->h_length);
    sfd2 = socket(PF_INET, SOCK_STREAM, 0);
    connect(sfd2, (struct sockaddr*)&saddr2, sizeof(saddr2));
    
    write(sfd2, buf, sizeof(buf));
    printf("wyslano do: %s wiadomosc: %s\n", sendTo, buf);
    close(sfd2);
}


int main(int argc, char * argv[]) {
    sendTo = argv[1];
    
    int sfd, cfd, on = 1;
    socklen_t sl;
    struct sockaddr_in saddr, caddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(1234);
    sfd = socket(PF_INET, SOCK_STREAM, 0);
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) 
        printf("error");
    bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr));
    listen(sfd, 5);
    while(1) {
        sl = sizeof(caddr);
        cfd = accept(sfd, (struct sockaddr*)&caddr, &sl);
        read(cfd, buf, sizeof(buf));
        printf("otrzymalem: %s\n", buf);
        sendToNextNode();
        close(cfd);
    }
    return 0;
}
