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
#include <pthread.h>

#define PORT  8000

struct sockaddr_in servaddr;

//thread that sends messages to the server
void *sender(void *arg);


// Driver code
int main(int argc, char *argv[]) {
    int i;
    int sockfd;
    char buffer[MAX_PACKET_SIZE];
	char auxarg[MAX_PACKET_SIZE];
    
    
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
    servaddr.sin_addr = *((struct in_addr *)server->h_addr);

    int n;
    socklen_t len = sizeof(servaddr);

    // Filling packet for test
    packet sentPacket;
    sentPacket.type = 5;
    sentPacket.seqn = 100;
    sentPacket.length = 42;
    sentPacket.total_size = 7;
    strcpy(sentPacket._payload, "PRA QUE MARSHLING NESSA CACETA ENRICAO");
    sentPacket.checksum = checkSum(&sentPacket);

    fflush(stdout);
    memcpy(buffer, &sentPacket, sizeof(buffer));

    
    
    sendto(sockfd, (const void *) buffer, MAX_PACKET_SIZE, MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr));  // Precisa arrumar o tamanho do que ta enviando
    printf("Packet sent.\n");                                                                                                     // 70 é só um numero cabalistico
    fflush( stdout );


    //prepara para enviar argumento para nova thread
    struct socketInfo hostInfo;
    hostInfo.sockfd = sockfd;
    hostInfo.servaddr = servaddr;
    hostInfo.len = len;

	memcpy(auxarg, &hostInfo, sizeof(hostInfo));
    //cria thread que envia
    pthread_t threadSender;
    pthread_create(&threadSender, NULL, sender, (void*) auxarg);

    close(sockfd);
    while(1)
       int juca = 1;
    return 0;
}


void *sender(void *arg) {
    printf("Thread is listening!\n");

    char buffer[MAX_PACKET_SIZE];
    socklen_t len = sizeof(servaddr);
	int n,sockfd;
	
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    n = recvfrom(sockfd, (char *)buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *) &servaddr, &len);
    if (n  < 0)
        printf("\nERROR on rcvfrom\n");
        
    printf("Server : %s\n", buffer);
    fflush( stdout );
}
