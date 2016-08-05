/*
 *
 * Compilation:  gcc -Wall tcp-sender-tester.c -o tcp-sender-tester
 * Usage:        ./tcp-sender-tester SERVER PORT
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h> 
#include <arpa/inet.h>

#define DURATION 30 /* seconds */
#define BUF_SIZE 100 /* seconds */

int main(int argc, char** argv) {
  time_t start = time(NULL), stop;
  unsigned long numOfBytesSent = 0;
  
  int sfd, rc;
  char buf[BUF_SIZE];
  int i;
  for (i=0; i<BUF_SIZE; i++)
      buf[i] = '0';
  struct sockaddr_in saddr;
  struct hostent* addrent;

  addrent = gethostbyname(argv[1]);
  sfd = socket(PF_INET, SOCK_STREAM, 0);
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(atoi(argv[2]));
  memcpy(&saddr.sin_addr.s_addr, addrent->h_addr, addrent->h_length);
  connect(sfd, (struct sockaddr*) &saddr, sizeof(saddr));
  printf("%s\n", buf);

  printf("start @ %s\n", ctime(&start));
  while(time(NULL) < start + DURATION) {
    numOfBytesSent += write(sfd, buf, BUF_SIZE);
  }
  stop = time(NULL);
  printf("stop  @ %s \nnumOfBytesSent %lu\n", ctime(&stop), numOfBytesSent);
  
  close(sfd);
  return EXIT_SUCCESS;
}
