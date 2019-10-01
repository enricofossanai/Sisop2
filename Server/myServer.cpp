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
user Users [MAXNUMCON];

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

    vector<pthread_t> threadsS(MAXNUMCON);              ////////////////////////////////////////////
    vector<pthread_t> threadsR(MAXNUMCON);              // Um vetor para cada thread diferente?? //
    int cliNum = 0;                                     ///////////////////////////////////////////
    int rc1,rc2;



    while(1){
        packet packetBuffer;
        int n, i;
        socklen_t len = sizeof(servaddr);

        n = recvfrom(sockfd, reinterpret_cast<void *> (&packetBuffer), MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        if (n < 0)
            printf("Error recvfrom\n");

        if(!(checkSum(&packetBuffer)))		    // Verificação de CheckSum
            perror("Verification failed");
        else{

            if(packetBuffer.type == CN){                                 //Testa se é o firstConnect

                user client;

                strcpy(client.username, packetBuffer._payload);
                client.cliaddr = cliaddr;

                Users[cliNum] = client;                                  // Vetor de Usuários para testar se já ta conectado

                rc1 = pthread_create(&threadsS[cliNum], NULL, cliThread, reinterpret_cast<void *> (&client));
                //rc2 = pthread_create(&threadsR[cliNum], NULL, receiver, reinterpret_cast<void *> (&cliaddr));
                cliNum++;
                }
        }
        fflush(stdout);
    }

    return 0;
}

void *cliThread(void *arg) {                           // Cuida dos Clientes
    int sockfd, sendToError;
    char buffer[MAXLINE];
    user *client;
    struct sockaddr_in servaddr;

	curPort++;						                   // Lembrar que é global, protege ou não?

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    client = reinterpret_cast<user *> (arg);

    printf("Thread do Cliente : %s\n", client->username);

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

    sendToError = sendto(sockfd, "É os guri ou não é?????\n", 50, 0,(const struct sockaddr *) &(client->cliaddr), sizeof(struct sockaddr));
    if (sendToError  < 0)
        printf("ERROR on sendto\n");

    fflush( stdout );
}


void *sender(void *arg) {
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
