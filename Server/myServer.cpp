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

    vector<pthread_t> threadsS(MAXNUMCON);              ///////////////////////////////////////////
    vector<pthread_t> threadsR(MAXNUMCON);              // Um vetor para cada thread diferente??
    int threadNum = 0;                                  //////////////////////////////////////////
    int rc1,rc2;

    while(1){
            packet packetBuffer;
            int n, i;
            socklen_t len = sizeof(servaddr);

            n = recvfrom(sockfd, reinterpret_cast<void *> (&packetBuffer), MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);

            if(!(checkSum(&packetBuffer)))		// Verificação de CheckSum
                {
                perror("Verification failed");
                exit(EXIT_FAILURE);                  // Tem que descartar o pacote
                }

            printf("Tipo: %d\n", packetBuffer.type);
            printf("Seq: %d\n",packetBuffer.seqn );
            printf("Len: %d\n",packetBuffer.length );
            printf("Size: %d\n",packetBuffer.total_size);
            printf("Check: %d\n", packetBuffer.checksum );
            printf("Payload: %s\n",packetBuffer._payload);


            char* userName = (char *) malloc(sizeof(char)*10);
            rc1 = pthread_create(&threadsS[threadNum], NULL, sender, reinterpret_cast<void *> (&cliaddr));
            rc2 = pthread_create(&threadsR[threadNum], NULL, receiver, reinterpret_cast<void *> (&cliaddr));
			threadNum++;
    }

    return 0;
}

void *sender(void *arg) {                           // Cuida dos Send para os Clientes
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




void *receiver(void *arg){                              // Cuida dos Receive dos Clients
}
/*    char buffer[MAXLINE];
    int n;
    socklen_t len = sizeof(servaddr);

    while(1){
        n = recvfrom(sockfd, buffer , MAXLINE, MSG_WAITALL, ( struct sockaddr *) arg, &len);
        printf("\n\nTeste: %s\nVim da porta: %d\n", buffer, servaddr.sin_port );
        fflush(stdout);
        }
}

*/
