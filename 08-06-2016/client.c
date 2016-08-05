/*
 *
 * Compilation:  gcc -Wall ./client.c -o ./client
 * Usage:        ./client SERVER PORT SEC
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
#include <time.h>
#include <unistd.h>

int _read(int fd, char *buf, int len) {
  int rc, total = 0;
  while (len > 0) {
    rc = read(fd, buf, len);
    len -= rc;
    buf += rc;
    total += rc;
  }
  return total;
}

int _write(int fd, char *buf, int len){
  int rc, total = 0;
  while (len > 0) {
    rc = write(fd, buf, len);
    len -= rc;
    buf += rc;
    total += rc;
  }
  return total;
}

int main(int argc, char** argv) {
  long double n = 0;
  int sfd, rc = 0;
  char buf[4096];
  struct sockaddr_in saddr;
  struct hostent* addrent;
  time_t start, end, loop, seconds = (time_t) atoi(argv[3]);

  addrent = gethostbyname(argv[1]);
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(atoi(argv[2]));
  memcpy(&saddr.sin_addr.s_addr, addrent->h_addr, addrent->h_length);
  start = time(NULL);
  loop = start;
  end = start + seconds;
  do {
    sfd = socket(PF_INET, SOCK_STREAM, 0);
    connect(sfd, (struct sockaddr*) &saddr, sizeof(saddr));
    rc = _write(sfd, buf, 256);
    rc = _read(sfd, buf, 4096);
    close(sfd);
    n += 1;
    loop = time(NULL);
  } while (loop < end);
  printf("%ld s\t%.0Lf r\t%.2Lf r/s\n", end - start, n, n / atoi(argv[3]));
  return EXIT_SUCCESS;
}
