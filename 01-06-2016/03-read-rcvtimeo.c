/*
 * Compilation:  gcc -Wall ./03-read-rcvtimeo.c -o ./03-read-rcvtimeo
 * Usage:        ./03-read-rcvtimeo SERVER PORT
 *
 *
 */

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int setitout(int sfd, int tout) {
  struct timeval tv;
  tv.tv_sec = tout;
  tv.tv_usec = 0;
  return setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

int main(int argc, char** argv) {
  int sfd, rc = 0;
  char buf[128];
  struct sockaddr_in saddr;
  struct hostent* addrent;

  addrent = gethostbyname(argv[1]);
  sfd = socket(PF_INET, SOCK_STREAM, 0);
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(atoi(argv[2]));
  memcpy(&saddr.sin_addr.s_addr, addrent->h_addr, addrent->h_length);
  connect(sfd, (struct sockaddr*) &saddr, sizeof(saddr));
  setitout(sfd, 1);
  rc = read(sfd, buf, 128);
  if (rc == -1)
    printf("[RCVTIMEO] timeout\n");
  write(1, buf, rc);
  close(sfd);
  return EXIT_SUCCESS;
}
