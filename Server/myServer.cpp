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

<<<<<<< HEAD
#define PORT        8000
#define MAXLINE     102400
#define MAXNUMCON   100

//header da thread foda-se
void *connect(void *arg);

=======
int curPort = 8000;
user Users [MAXNUMCON];
>>>>>>> c2d020fbcaafa0b48a212d92884d35d9d7b76d10

// Driver code
int main() {
    int sockfd;
    char buffer[MAX_PAYLOAD_SIZE];
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

        n = recvfrom(sockfd, reinterpret_cast<void *> (&packetBuffer), MAX_PACKET_SIZE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        if (n < 0)
            printf("Error recvfrom\n");

        if(!(checkSum(&packetBuffer)))		    // Verificação de CheckSum
            perror("Verification failed");
        else{

            if(packetBuffer.type == CN){                                 //Testa se é o firstConnect

                user client;

<<<<<<< HEAD
            printf("Tipo: %d\n", packetBuffer.type);
            printf("Seq: %d\n",packetBuffer.seqn );
            printf("Len: %d\n",packetBuffer.length );
            printf("Size: %d\n",packetBuffer.total_size);
            printf("Check: %d\n", packetBuffer.checksum );
            printf("Payload: %s\n",packetBuffer._payload);

            char* userName = "jucaBatista";
            rc = pthread_create(&threads[threadNum], NULL, connect, (void*)cliaddr);
=======
                strcpy(client.username, packetBuffer._payload);
                client.cliaddr = cliaddr;

                Users[cliNum] = client;                                  // Vetor de Usuários para testar se já ta conectado

                curPort ++;
                client.socket = createSocket(client, curPort);


                rc1 = pthread_create(&threadsS[cliNum], NULL, cliThread, reinterpret_cast<void *> (&client));
                //rc2 = pthread_create(&threadsR[cliNum], NULL, receiver, reinterpret_cast<void *> (&cliaddr));
                cliNum++;
                }
        }
        fflush(stdout);
>>>>>>> c2d020fbcaafa0b48a212d92884d35d9d7b76d10
    }

    return 0;
}

void *cliThread(void *arg) {                           // Cuida dos Clientes
    int n;
    char buffer[MAX_PAYLOAD_SIZE];
    user *client;
    packet sendPacket;
    packet recPacket;
    socklen_t len = sizeof(struct sockaddr_in);

    client = reinterpret_cast<user *> (arg);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    while (1){

        n = recvfrom(client->socket, reinterpret_cast<void *> (&recPacket), MAX_PACKET_SIZE, 0, ( struct sockaddr *)  &(client->cliaddr), &len);
        if (n < 0)
            perror("recvfrom");

        printf("Length : %d\nRecebido de : %s\nPayload : %s\n\n", recPacket.length, client->username, recPacket._payload);
        fflush( stdout );

        n = receiveFile("revistaJuca" , recPacket.length, client->cliaddr, client->socket);

        if (recPacket.type == CMD){
            switch (recPacket.cmd) {
                case UPLOAD:;
                // receiveFile()                     // Lembrar do // nos pathname!!!!!
                // Temo que receber o arquivo do cliente
                case DOWNLOAD:;
                // sendFile()
                //Mesma coisa do UPLOAD, só que server -> client
                case DELETE:;
                // delete (recPacket._payload)
                // Recebe o nome do arquivo, apaga da base do Servidor

            }
        }
    }
}


void *sender(void *arg) {
}
