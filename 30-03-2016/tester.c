/*
 *
 * Compilation:  gcc -Wall ./timeloop.c -o ./timeloop
 * Usage:        ./timeloop
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define DURATION 30 /* seconds */

int main(int argc, char** argv) {
  time_t start = time(NULL), stop;

  printf("start @ %s", ctime(&start));
  while(time(NULL) < start + DURATION) {
    sleep(1); /* your code here */
  }
  stop = time(NULL);
  printf("stop  @ %s", ctime(&stop));
  return EXIT_SUCCESS;
}
