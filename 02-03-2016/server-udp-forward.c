#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char buf[128];
char *sendTo;

void sendToNextNode() {
	printf("wysylanie: %s\n", buf);
	int sfd2;
	struct sockaddr_in saddr2, caddr2;
	struct hostent* addrent2;
	addrent2 = gethostbyname(sendTo);
	memset(&caddr2, 0, sizeof(caddr2));
	caddr2.sin_family = AF_INET;
	caddr2.sin_addr.s_addr = INADDR_ANY;
	caddr2.sin_port = 0;
	memset(&saddr2, 0, sizeof(saddr2));
	saddr2.sin_family = AF_INET;
	memcpy(&saddr2.sin_addr.s_addr, addrent2->h_addr, addrent2->h_length);
	saddr2.sin_port = htons(1234);
	sfd2 = socket(PF_INET, SOCK_DGRAM, 0);
	bind(sfd2, (struct sockaddr*)&caddr2, sizeof(caddr2));
	sendto(sfd2, buf, strlen(buf)+1, 0, (struct sockaddr*)&saddr2, sizeof(saddr2));
	printf("wyslano: %s\n", buf);
	close(sfd2);
}


int main(int argc, char * argv[]) {
	sendTo = argv[1];
	int sfd; socklen_t sl;
	struct sockaddr_in saddr, caddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(1234);
	sfd = socket(PF_INET, SOCK_DGRAM, 0);
	bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr));
	while(1) {
		memset(&caddr, 0, sizeof(caddr));
		memset(buf, 0, sizeof(buf));
		sl = sizeof(caddr);
		recvfrom(sfd, buf, 128, 0, (struct sockaddr*)&caddr, &sl);
		printf("odebrano: %s\n", buf);
		sendToNextNode();
	}
	return 0;
}
