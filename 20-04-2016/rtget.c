/*
 *
 * Compilation:  gcc -Wall ./rtget.c -o ./rtget
 * Usage:        ./rtget
 *
 *
 */

#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct reqhdr {
  struct nlmsghdr nl;
  struct rtmsg    rt;
};

int main(int argc, char **argv) {
  int sfd, rclen, nllen, atlen;
  char *ptr;
  char buf[8192];
  char dst[32], msk[32], gwy[32], dev[32];
  struct sockaddr_nl snl;
  struct reqhdr req;
  struct nlmsghdr *nlp;
  struct rtmsg *rtp;
  struct rtattr *atp;

  sfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  memset(&snl, 0, sizeof(struct sockaddr_nl));
  snl.nl_family = AF_NETLINK;
  snl.nl_pid = 0;
  memset(&req, 0, sizeof(struct reqhdr));
  req.nl.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  req.nl.nlmsg_type = RTM_GETROUTE;
  req.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;
  req.nl.nlmsg_seq = 0;
  req.nl.nlmsg_pid = getpid();
  req.rt.rtm_family = AF_INET;
  req.rt.rtm_table = RT_TABLE_MAIN;
  sendto(sfd, (void*) &req, sizeof(struct reqhdr), 0, (struct sockaddr*) &snl,
         sizeof(struct sockaddr_nl));

  memset(&buf, 0, sizeof(buf));
  ptr = buf;
  nllen = 0;
  do {
    rclen = recv(sfd, ptr, sizeof(buf) - nllen, 0);
    nlp = (struct nlmsghdr*) ptr;
    ptr += rclen;
    nllen += rclen;
  } while(nlp->nlmsg_type == NLMSG_DONE);

  nlp = (struct nlmsghdr*) buf;
  for(;NLMSG_OK(nlp, nllen); nlp = NLMSG_NEXT(nlp, nllen)) {
    rtp = (struct rtmsg*) NLMSG_DATA(nlp);
    /*if(rtp->rtm_table != RT_TABLE_MAIN)
      continue;*/
    atp = (struct rtattr*) RTM_RTA(rtp);
    atlen = RTM_PAYLOAD(nlp);
    memset(dst, 0, sizeof(dst));
    memset(msk, 0, sizeof(msk));
    memset(gwy, 0, sizeof(gwy));
    memset(dev, 0, sizeof(dev));
    //printf("nllen %d\n", nllen);
    for(;RTA_OK(atp, atlen); atp = RTA_NEXT(atp, atlen)) {
        //printf("atlen %d\n", atlen);
      switch(atp->rta_type) {
        case RTA_DST:     inet_ntop(AF_INET, RTA_DATA(atp), dst, sizeof(dst));
                          break;
        case RTA_GATEWAY: inet_ntop(AF_INET, RTA_DATA(atp), gwy, sizeof(gwy));
                          break;
        case RTA_OIF:     sprintf(dev, "%d", *((int*) RTA_DATA(atp)));
                          break;
      }
    }
    sprintf(msk, "%d", rtp->rtm_dst_len);
    if (strlen(dst) == 0)
      printf("default via %s dev %s\n", gwy, dev);
    else if (strlen(gwy) == 0)
      printf("%s/%s dev %s\n", dst, msk, dev);
    else
      printf("dst %s/%s gwy %s dev %s\n", dst, msk, gwy, dev);
  }

  close(sfd);
  return EXIT_SUCCESS;
}
