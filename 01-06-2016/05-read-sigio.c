/*
 *
 * Compilation:  gcc -Wall ./05-read-sigio.c -o ./05-read-sigio
 * Usage:        ./05-read-sigio SERVER PORT
 *
 *
 */

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int sfd, rc = -1;
char buf[128];

void callback(int signo) { if (read(sfd, buf, 128) > 0) rc = 1; }

int main(int argc, char** argv) {
  int flags;
  struct sockaddr_in saddr;
  struct hostent* addrent;

  addrent = gethostbyname(argv[1]);
  sfd = socket(PF_INET, SOCK_STREAM, 0);
  fcntl(sfd, F_SETOWN, getpid());
  fcntl(sfd, F_SETSIG, SIGIO);
  flags = fcntl(sfd, F_GETFL, 0);
  fcntl(sfd, F_SETFL, flags | O_NONBLOCK | O_ASYNC); //O_ASYNC - do danego procesu zostanie wyslany sygnal ze moze odebrac wiadomosc
  signal(SIGIO, callback);
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(atoi(argv[2]));
  memcpy(&saddr.sin_addr.s_addr, addrent->h_addr, addrent->h_length);
  connect(sfd, (struct sockaddr*) &saddr, sizeof(saddr));
  while (rc < 0) printf("waiting...\n");
  write(1, buf, strlen(buf) + 1);
  close(sfd);
  return EXIT_SUCCESS;
}
