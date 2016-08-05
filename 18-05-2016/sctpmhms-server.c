#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define LOCAL_TIME_STREAM 0
#define GREENWICH_MEAN_TIME_STREAM 1

int main(int argc, char **argv) {
  int sfd, cfd, i, on = 1, no = argc - 1;
  socklen_t sl = sizeof(struct sockaddr_in);
  struct sockaddr_in saddr, caddr, addr, *addrs;
  char buf[1024];
  time_t now;

  sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP);
  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));

  addrs = malloc(sl * no);
  for(i = 1; i < argc; i++) {
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(argv[i]);
    saddr.sin_port = htons(1234);
    memcpy(addrs + (i-1), &saddr, sl);
  }
  sctp_bindx(sfd, (struct sockaddr*) addrs, no, SCTP_BINDX_ADD_ADDR);
  no = sctp_getladdrs(sfd, 0, (struct sockaddr**) &addrs);
  printf("%d\n", no);
  for (i = 0; i < no; i++) {
    memcpy(&addr, addrs + i, sl);
    printf("%d: %s:%d\n", i, inet_ntoa(addr.sin_addr), addr.sin_port);
  }

  listen(sfd, 5);
  while(1) {
    memset(&caddr, 0, sizeof(caddr));
    sl = sizeof(caddr);
    cfd = accept(sfd, (struct sockaddr*) &caddr, &sl);
    printf("new connection: %s:%d\n", inet_ntoa(caddr.sin_addr),
           caddr.sin_port);
    while(1) {
        now = time(NULL);
        snprintf(buf, 1024, "%s", ctime(&now));
        sctp_sendmsg(cfd, (void*) buf, (size_t) strlen(buf) + 1, NULL, 0, 0, 0,
                    LOCAL_TIME_STREAM, 0, 0);
        snprintf(buf, 1024, "%s", asctime(gmtime(&now)));
        sctp_sendmsg(cfd, (void*) buf, (size_t) strlen(buf) + 1, NULL, 0, 0, 0,
                    GREENWICH_MEAN_TIME_STREAM, 0, 0);
        sleep(1);
    }
    close(cfd);
  }
  close(sfd);
  return EXIT_SUCCESS;
}
