#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

int main(int argc, char** argv) {
    printf("use with sudo\n");
    int sfd;
    struct ifreq ifr;
    
    sfd = socket(PF_INET, SOCK_DGRAM, 0);
    strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);
    
    ioctl(sfd, SIOCGIFFLAGS, &ifr);

    if(strcmp(argv[2], "u") == 0) {
        ifr.ifr_flags |= IFF_UP;
        ioctl(sfd, SIOCSIFFLAGS, &ifr);
        printf("%s is now up\n", argv[1]);
    }
    else if(strcmp(argv[2], "d") == 0) {
        ifr.ifr_flags &= ~IFF_UP;
        ioctl(sfd, SIOCSIFFLAGS, &ifr);
        printf("%s is now down\n", argv[1]);
    }
    else {
        printf("nothing changed\n");
    }
    return 0;
}
