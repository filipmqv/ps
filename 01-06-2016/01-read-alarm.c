/*
 * 
 * Compilation:  gcc -Wall ./01-read-alarm.c -o ./01-read-alarm
 * Usage:        ./01-read-alarm SERVER PORT
 *
 *
 */

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define TIMEOUT 1

void callback(int signo) { printf("[SIGALRM] timeout\n"); return; }

int _read(int sfd, char *buf, int nbytes, int tout) {
  int r;
  sig_t sig;
  siginterrupt(SIGALRM, 1);
  sig = signal(SIGALRM, callback);
  alarm(tout);
  r = read(sfd, buf, nbytes);
  if (errno == EINTR) errno = ETIMEDOUT;
  alarm(0);
  siginterrupt(SIGALRM, 0);
  signal(SIGALRM, sig);
  return r;
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
  rc = _read(sfd, buf, 128, TIMEOUT);
  write(1, buf, rc);
  close(sfd);
  return EXIT_SUCCESS;
}
