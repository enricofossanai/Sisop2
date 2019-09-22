//Headers
#include "aplication.h"
#include "communication.h"
#include "sync.h"

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

#define PORT     8000
#define MAXLINE 102400
#define MAX_PACKET_SIZE 64000

// Driver code
int main(int argc, char *argv[]) {
    int sockfd, i;
    char buffer[MAXLINE];
    char *hello = "Hello from client";
    struct sockaddr_in servaddr;
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

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    int n;
    socklen_t len = sizeof(servaddr);

  
    packet sentPacket;
    sentPacket.type = 5;
    sentPacket.seqn = 100;
    sentPacket.length = 42;
    sentPacket.total_size = 7;
    strcpy(sentPacket._payload, "PRA QUE MARSHLING NESSA CACETA ENRICAO");
    sentPacket.checksum = checkSum(&sentPacket);

    fflush(stdout);
    memcpy(buffer, &sentPacket, sizeof(buffer));


    char * message = "conectando";
 

    sendto(sockfd, (const void *) buffer, MAX_PACKET_SIZE, MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr));  // Precisa arrumar o tamanho do que ta enviando
    printf("Packet sent.\n");                                                                                         // 70 é só um numero cabalistico
    fflush( stdout );

    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);

    printf("Server : %s\n", buffer);
    fflush( stdout );

    close(sockfd);
    return 0;
}
