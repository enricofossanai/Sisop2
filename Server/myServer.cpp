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

user Users [MAXNUMCON];
int curPort = 8000;

#define PORT        8000
#define MAXLINE     102400
#define MAXNUMCON   100


userList* head = (userList*)malloc(sizeof(userList));

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


    vector<pthread_t> threads(MAXNUMCON);              // Um vetor para cada thread diferente?? //
    int cliNum = 0;
    int rc1;

    //userList* head = (userList*)malloc(sizeof(userList));
    head->next = NULL;

    while(1){
        packet packetBuffer;
        int n, i;
        socklen_t len = sizeof(servaddr);
	pthread_t new_thread;

        n = recvfrom(sockfd, reinterpret_cast<void *> (&packetBuffer), MAX_PACKET_SIZE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
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

                curPort ++;
                client.socket = createSocket(client, curPort);


                //Adicionando cliente a lista de usuoarios conectados
                addToONlist (&head, &client);
                displayList(head);

                rc1 = pthread_create(&new_thread, NULL, cliThread, reinterpret_cast<void *> (&client));
                cliNum++;
		threads.push_back(new_thread);
                }
        }
        fflush(stdout);
    }

    return 0;
}

void *cliThread(void *arg) {                           // Cuida dos Clientes
    printf("\nCriada a thread do cliente");
    fflush(stdout);
    int n;
    char buffer[MAX_PAYLOAD_SIZE];
    user *client;
    packet sendPacket;
    packet recPacket;
    socklen_t len = sizeof(struct sockaddr_in);
    cmdAndFile lastCommand;
    char dirClient[200] = {};
    char file[100] = {};


    client = reinterpret_cast<user *> (arg);

    strcpy(dirClient, "./");
    strcat(dirClient, client->username);
    strcat(dirClient, "/");


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    while (1){
        bzero(file, 100);
        strcpy(file, dirClient);


        lastCommand = rcv_cmd(client->cliaddr,client->socket);

        //printf("\nserver recieved command %d from %s", recPacket.type , recPacket._payload );

        if (lastCommand.command >= 0){ // if recieved command wasnt corrupted
            if(lastCommand.command == CREATE) {
                printf("\nRECIEVED CREATE FILE COMMAND");
                strcat(file, lastCommand.fileName);
                printf("FILE : %s\n", file);
                n =  receiveFile( file , lastCommand.fileSize, client->cliaddr,client->socket );    // Lembrar do // nos pathname!!!!!
                // Temo que receber o arquivo do cliente
            }
            else if(lastCommand.command == DELETE) {
                printf("\nRECIEVED DELETE FILE COMMAND");
                n = delete_file(lastCommand.fileName,client->username);
              }
            else if (lastCommand.command == MODIFY){
                printf("\nRECIEVED MODIFY FILE COMMAND");
                // delete (recPacket._payload)
                // Recebe o nome do arquivo, apaga da base do Servidor
              }
              else if (lastCommand.command ==LIST_SERVER){
                printf("\nRECIEVED LIST_SERVER COMMAND");
                    if (list_server(client->username, buffer)){
                        fflush(stdout);
                        strcpy(sendPacket._payload,buffer);
                        sendPacket.type = DATA;
                        sendPacket.checksum = makeSum(&sendPacket);
                        printf("\nEnviando lista de arquivos\n");
                        n = sendto(client->socket, reinterpret_cast<void *> (&sendPacket), MAX_PACKET_SIZE, 0, ( struct sockaddr *)  &(client->cliaddr), sizeof(client->cliaddr));
                        if (n  < 0)
                            perror("sendto");
                        fflush(stdout);

                    }
              }
              else if (lastCommand.command == EXIT){
                printf("\nRECIEVED LIST_SERVER EXIT");
                rmvFromONlist (&head, client);
                displayList(head);
              }
              else if (lastCommand.command == DOWNLOAD){
                  printf("\nRECIEVED UPLOAD COMMAND");
                  strcat(file, lastCommand.fileName);
                  printf("FILE : %s\n", file);
                  n =  sendFile( file , client->cliaddr,client->socket );

              }

          }
      }

}
