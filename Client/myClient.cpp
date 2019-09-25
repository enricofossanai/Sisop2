//Headers
#include "aplication.h"
#include "commClient.h"

// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

struct sockaddr_in servaddr;
int sockfd;

// Driver code
int main(int argc, char *argv[]) {
    int i;
    char buffer[MAX_PACKET_SIZE];

    struct hostent *server;

    if (argc < 2) {
		fprintf(stderr, "usage %s hostname\n", argv[0]);
		exit(0);

	}

    server = gethostbyname(argv[1]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

	firstConnect(sockfd,server);

    //cria thread que envia
    pthread_t threadSender;
    pthread_create(&threadSender, NULL, sender, NULL);

    while(1)
       int juca = 1;

    close(sockfd);

    return 0;
}

void *sender(void *arg) {
    printf("Thread is listening!\n");

    char buffer[MAX_PACKET_SIZE];
    socklen_t len = sizeof(servaddr);
	int n;

    n = recvfrom(sockfd, (char *)buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *) &servaddr, &len);
    if (n  < 0)
        printf("\nERROR on rcvfrom\n");

    printf("Server : %s\n", buffer);
    fflush( stdout );
}

