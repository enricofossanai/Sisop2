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
int curPort = 8002;                             // 8000 Servidor / 8001 Election
int primary = 0;
int eleNum = -1;                                // FLAG DE PRIMARIO


#define PORT        8000
#define BACKUPORT   7000                        // Só pra testes em mesma máquina pra evitar conflito
#define MAXLINE     102400
#define MAXNUMCON   100


//userList* head = (userList*)malloc(sizeof(userList));
user uList[10] = { { 0 } };

struct sockaddr_in serverlist [10];
struct sockaddr_in electlist [10];

struct hostent *firstser;

// Driver code
int main(int argc, char *argv[]) {
    int sockfd;
    char buffer[MAX_PAYLOAD_SIZE];
    struct sockaddr_in servaddr, cliaddr, addr;
    user client;

    if (argc < 3) {
        fprintf(stderr, "usage %s id primaryname\nPrimary server id = 0   name = 0\n", argv[0]);
        exit(0);
    }

    if(strcmp(argv[1], "0") == 0)
        primary = 1;

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
    if(primary == 1)
    	servaddr.sin_port = htons(PORT);
	else
		servaddr.sin_port = htons(BACKUPORT);


    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (primary == 0){
        firstser = gethostbyname(argv[2]);
        connectBackup(sockfd, firstser, CS);
    }

    int cliNum = 0;
    int servNum = -1;
    int rc1;

    //head->next = NULL;

/////////////////////////// CRIANDO MAIS UMA THREAD: (ALIVE + ELECTION)
    pthread_t telection;

    rc1 = pthread_create(&telection, NULL, election, NULL);
    if(rc1 < 0)
        perror("pthread_create");
///////////////////////////////////////////////////

    while(1){
        packet packetBuffer;
        int n, i;
        socklen_t len = sizeof(servaddr);
	    pthread_t tid[100];

        memset(&packetBuffer, 0 , sizeof(struct packet));

        n = recvfrom(sockfd, reinterpret_cast<void *> (&packetBuffer), MAX_PACKET_SIZE, MSG_WAITALL, ( struct sockaddr *) &addr, &len);
        if (n < 0)
            printf("Error recvfrom\n");

	if (primary == 0){

	  // USAR PARA COMUNICAÇÃO DOS BACKUPS COM O PRIMARIO
      // FAZER UMA FUNÇÂO QUE TRATE ESSE RECEIVE (CREATE DELETE MODIFY)
      // NÂO ENCHER ESSE IF DE SWITCH CASE

	}

        if(!(checkSum(&packetBuffer)))		    // Verificação de CheckSum
            perror("Verification failed");
        else{

            if(packetBuffer.type == CN){                                 //Conexao de Cliente

                memset(&client, 0 , sizeof(struct user));

                strcpy(client.username, packetBuffer._payload);
                client.cliaddr = addr;

                curPort ++;


                client.socket = createSocket(client, curPort);
                client.cliSend = getClientLSocket(client, client.socket);

                Users[cliNum] = client;

                //Adicionando cliente a lista de usuoarios conectados
                /*
                addToONlist (uList, client);
                displayList(uList);
*/

                rc1 = pthread_create(&tid[cliNum], NULL, cliThread, reinterpret_cast<void *> (&Users[cliNum]) );
                if(rc1 < 0)
                    perror("pthread_create");

                rc1 = pthread_detach(tid[cliNum]);
                if(rc1 < 0)
                    perror("pthread_detach");

                cliNum++;

                }

            if(packetBuffer.type == CS){                    // Conexão de Server Backup
                servNum ++;
                serverlist[servNum] = addr;

                n = sendto(sockfd, reinterpret_cast<void *> (&packetBuffer), MAX_PACKET_SIZE, 0, ( struct sockaddr *)  &addr, sizeof(addr));
                if (n  < 0)
                    perror("sendto");
                printf("ENTREI AQUI CUPIXA\n");
            }

            if(packetBuffer.type == CE){
                electlist[eleNum + 1] = addr;
                eleNum++;

                n = sendto(sockfd, reinterpret_cast<void *> (&packetBuffer), MAX_PACKET_SIZE, 0, ( struct sockaddr *)  &addr, sizeof(addr));
                if (n  < 0)
                    perror("sendto");
                printf("ENTREI AQUI CUPIXA 2\n");
            }
        }
        fflush(stdout);
    }

    return 0;
}

void *cliThread(void *arg) {                                                    // Cuida dos Clientes
    user *client;
    cmdAndFile lastCommand;
    char dirClient[100] = {};


    client = reinterpret_cast<user *> (arg);

    printf("\nCriada a thread do cliente : %s\n", client->username);

    strcpy(dirClient, "./");
    strcat(dirClient, client->username);
    strcat(dirClient, "/");

    while (1){
        lastCommand = rcv_cmd(client->cliaddr,client->socket);

        printf("\nserver received command %d from %s\n", lastCommand.command ,client->username);

        if (lastCommand.command >= 0) // if received command wasnt corrupted
            make_cmd(lastCommand, client, dirClient, uList);
    }

}

void *election (void *arg){
    int size = sizeof(struct sockaddr_in);
    int n;
    int i = 0;
    int socksd;
    struct sockaddr_in servaddr, send;
    packet packet;

    printf("THREAD DA ELEIÇÂO\n");

    if ( (socksd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    if (primary == 1)
        servaddr.sin_port = htons(PORT + 1);
    else
        servaddr.sin_port = htons(BACKUPORT + 1);

    // Bind the socket with the server address
    if ( bind(socksd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    struct timeval timeout={2,0};                                                       //set timeout for 2 seconds
    setsockopt(socksd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));


    if (primary == 0)
        connectBackup(socksd, firstser, CE);


    while(1){
        if (primary == 1){                      // Se for o primario fica mandando ALIVE

            if(eleNum == -1)
                continue;

			// sendPacket._payload = lista de servers backup
            send = electlist[i];
            n = sendto(socksd, reinterpret_cast<void *> (&packet), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &(send), size);
            if(n < 0)
                perror("sendto");

            if(i < eleNum)
                i++;
            else
                i = 0;

        }
        else {         // Se for backup fica ouvindo

            n = recv(socksd, reinterpret_cast<void *> (&packet), MAX_PACKET_SIZE, 0);
            if (n  < 0)
                printf("ACHO QUE O VAGABUNDO MORREU\n");        // Aqui vai a eleição
        }
    }
}
