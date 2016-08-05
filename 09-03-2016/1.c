#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

int sfd;

struct ifconf getifreqs() {
    int len, lastlen;
    char* buf;
    struct ifconf ifc;
    lastlen = 0;
    len = 100 * sizeof(struct ifreq);
    sfd = socket(PF_INET, SOCK_DGRAM, 0);
    while(1) {
        buf = malloc(len);
        memset(buf, 0, len);
        ifc.ifc_len = len;
        ifc.ifc_buf = buf;
        if (ioctl(sfd, SIOCGIFCONF, &ifc) < 0) {
            if (errno != EINVAL || lastlen != 0)
                exit(-1);
        } else {
            if (ifc.ifc_len < len || ifc.ifc_len == lastlen)
                break;
            lastlen = ifc.ifc_len;
        }
        len += 10 * sizeof(struct ifreq);
        free(buf);
    }
    return ifc;
}

void ifsinfo(struct ifconf ifc) {
    char* ptr;
    struct ifreq* ifr;
    struct sockaddr_in* addr_in;
    ptr = ifc.ifc_buf;
    while(ptr < ifc.ifc_buf + ifc.ifc_len) {
        ifr = (struct ifreq*) ptr;
        ptr += sizeof(struct ifreq);
        printf("%s\t", ifr->ifr_name);
        addr_in = (struct sockaddr_in*)&ifr->ifr_addr;
        printf("inet addr:%s\t", inet_ntoa((struct in_addr)addr_in->sin_addr));
        
        
        ioctl(sfd, SIOCGIFHWADDR, ifr);
        const unsigned char* mac=(unsigned char*)ifr->ifr_hwaddr.sa_data;
        printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    }
}

int main(int argc, char** argv) {
    struct ifconf ifc = getifreqs();
    ifsinfo(ifc);
    free(ifc.ifc_buf);
    return 0;
}
