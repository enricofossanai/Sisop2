 //Headers
#include "commServer.h"
#include "fileManager.h"

// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <vector>


using namespace std;
int curPort = 8000;

// Driver code
int main() {
    int sockfd;
    char buffer[MAXLINE];
	char auxarg[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    vector<pthread_t> threads(MAXNUMCON);
    int threadNum = 0;
    int rc;

    while(1){
            packet packetBuffer;
            int n, i;
            socklen_t len = sizeof(servaddr);

            n = recvfrom(sockfd, reinterpret_cast<void *> (&packetBuffer), MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);

            if(!(checkSum(&packetBuffer)))		// Verificação de CheckSum
                {
                perror("Verification failed");
                exit(EXIT_FAILURE);
                }

            printf("Tipo: %d\n", packetBuffer.type);
            printf("Seq: %d\n",packetBuffer.seqn );
            printf("Len: %d\n",packetBuffer.length );
            printf("Size: %d\n",packetBuffer.total_size);
            printf("Check: %d\n", packetBuffer.checksum );
            printf("Payload: %s\n",packetBuffer._payload);


            char* userName = (char *) malloc(sizeof(char)*10);
            rc = pthread_create(&threads[threadNum], NULL, connect, reinterpret_cast<void *> (&cliaddr));
			threadNum++;
    }

    return 0;
}

void *connect(void *arg) {
    printf("\nConnection thread\n");
    fflush(stdout);
    int sockfd, sendToError;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
	
	curPort++;						// Lembrar que é global, protege ou não?

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(curPort);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    sendToError = sendto(sockfd, "É os guri ou não é?????\n", 50, 0,(const struct sockaddr *) arg, sizeof(struct sockaddr));
    if (sendToError  < 0)
        printf("ERROR on sendto\n");

    fflush( stdout );
}


