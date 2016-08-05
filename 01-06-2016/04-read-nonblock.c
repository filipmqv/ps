/*
 *
 * Compilation:  gcc -Wall ./04-read-nonblock.c -o ./04-read-nonblock
 * Usage:        ./04-read-nonblock SERVER PORT
 *
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
  int sfd, rc = 0, flags;
  char buf[128];
  struct sockaddr_in saddr;
  struct hostent* addrent;

  addrent = gethostbyname(argv[1]);
  sfd = socket(PF_INET, SOCK_STREAM, 0);
  flags = fcntl(sfd, F_GETFL, 0);
  fcntl(sfd, F_SETFL, flags | O_NONBLOCK);
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(atoi(argv[2]));
  memcpy(&saddr.sin_addr.s_addr, addrent->h_addr, addrent->h_length);
  connect(sfd, (struct sockaddr*) &saddr, sizeof(saddr));
  while (1) {
    printf("reading...\n");
    rc = read(sfd, buf, 128);
    perror("read()");
    errno = 0;
    if (rc > 0)
      break;
  }
  write(1, buf, rc);
  close(sfd);
  return EXIT_SUCCESS;
}
