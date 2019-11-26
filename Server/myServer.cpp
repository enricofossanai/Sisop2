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
int ID = 0;

#define PORT        8000
#define BACKUPORT   7000                        // Só pra testes em mesma máquina pra evitar conflito
#define MAXLINE     102400
#define MAXNUMCON   100


//userList* head = (userList*)malloc(sizeof(userList));
struct user uList[10];

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
    else
        ID = atoi(argv[1]);

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
		servaddr.sin_port = htons(BACKUPORT + (ID * 2));


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

    rc1 = pthread_create(&telection, NULL, election, reinterpret_cast<void *> (&servaddr));
    if(rc1 < 0)
        perror("pthread_create");
///////////////////////////////////////////////////

    while(1){
        packet packetBuffer;
        int n, i, j;
        socklen_t len = sizeof(servaddr);
	    pthread_t tid[100];
        struct sockaddr_in send;
        cmdAndFile lastCommand;
        char *username = (char *)malloc(sizeof(char) * 100);

        memset(&packetBuffer, 0 , sizeof(struct packet));

        n = recvfrom(sockfd, reinterpret_cast<void *> (&packetBuffer), MAX_PACKET_SIZE, MSG_WAITALL, ( struct sockaddr *) &addr, &len);
        if (n < 0)
            printf("Error recvfrom\n");

	if (primary == 0){
        username = backup_rcvd(packetBuffer, addr, sockfd);
        lastCommand = rcv_cmd(addr, sockfd);
        server_cmd(lastCommand, addr, username, sockfd);
        printf("Recebeu o comando: %d\n", lastCommand.command);
	}

        if(!(checkSum(&packetBuffer)))		    // Verificação de CheckSum
            perror("Verification failed");
        else{

            if(packetBuffer.type == CN){                                 //Conexao de Cliente

                cliNum = (curPort - 2) % 8000;

                memset(&client, 0 , sizeof(struct user));

                strcpy(client.username, packetBuffer._payload);
                client.cliaddr = addr;

                curPort ++;

                client.socket = createSocket(client, curPort);
                client.cliSend = getClientLSocket(client, client.socket);

                Users[cliNum] = client;

                //Adicionando cliente a lista de usuoarios conectados
                j = 0;
                while(j <= eleNum){
                    send = serverlist[j];
                    send_cmd(client.username, send, sockfd, NAME, NULL);
                    sleep(1);
                    send_cmd(client.username, send, sockfd, CLIENT, NULL);
                    j++;
                }

                addToONlist (uList, client);
                displayList(uList);

                rc1 = pthread_create(&tid[cliNum], NULL, cliThread, reinterpret_cast<void *> (&Users[cliNum]) );
                if(rc1 < 0)
                    perror("pthread_create");

                }

            if(packetBuffer.type == CS){                    // Conexão de Server Backup
                servNum ++;
                serverlist[servNum] = addr;

                n = sendto(sockfd, reinterpret_cast<void *> (&packetBuffer), MAX_PACKET_SIZE, 0, ( struct sockaddr *)  &addr, sizeof(addr));
                if (n  < 0)
                    perror("sendto");
            }

            if(packetBuffer.type == CE){
                electlist[eleNum + 1] = addr;
                eleNum++;

                n = sendto(sockfd, reinterpret_cast<void *> (&packetBuffer), MAX_PACKET_SIZE, 0, ( struct sockaddr *)  &addr, sizeof(addr));
                if (n  < 0)
                    perror("sendto");
                printf("Servidor backup nº%d conectado\n", eleNum + 1);
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

        printf("\nServer received command %d from %s\n", lastCommand.command ,client->username);

        if (lastCommand.command >= 0) // if received command wasnt corrupted
            make_cmd(lastCommand, client, dirClient, uList,serverlist, eleNum);
    }

}

void *election (void *arg){
    int size = sizeof(struct sockaddr_in);
    int n , j = 0;
    int i = 0;
    int socksd;
    int vote = 0;           // Se vote = 0 (Não entrou na votação)  se vote = 1 (Tá participante)
    struct sockaddr_in servaddr, send;
    packet packet;
    backupComm lists;
    struct sockaddr_in *mainaddr = reinterpret_cast<struct sockaddr_in *> (arg);

    //printf("THREAD DA ELEIÇÂO\n");

    if ( (socksd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    if (primary == 1)
        servaddr.sin_port = htons(PORT + 1);
    else
        servaddr.sin_port = htons(BACKUPORT + ID);

    // Bind the socket with the server address
    if ( bind(socksd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    struct timeval timeout={10,0};                                                       //set timeout for 2 seconds
    setsockopt(socksd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));


    if (primary == 0)
        connectBackup(socksd, firstser, CE);


    while(1){
        if (primary == 1){                      // Se for o primario fica mandando ALIVE

          if(eleNum == -1)
                continue;


            memcpy(lists.slist, serverlist, sizeof(lists.slist));
            memcpy(lists.elist, electlist, sizeof(lists.elist));
            memcpy(lists.uList, uList, sizeof(lists.uList));
            lists.eNum = eleNum;


            packet.type = ALIVE;
			memcpy(packet._payload, &lists, sizeof(lists));
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

            n = recvfrom(socksd, reinterpret_cast<void *> (&packet), MAX_PACKET_SIZE, 0, NULL , NULL);
            if (n  < 0){
                printf("ACHO QUE O VAGABUNDO MORREU: %d\n", eleNum);        // Aqui vai a eleição
                if (eleNum == 0){
                    primary = 1;                                            // Tá sozinho no rolê
                    printf("NOVO PRIMARIO 1\n");
                    deleteElement(serverlist, *mainaddr);
                    deleteElement(electlist, servaddr);
                    eleNum--;

                    curPort = send_cli(uList, socksd, curPort);
                }
                else
                    vote = makeElection(electlist,servaddr,ID,socksd, eleNum);
            }

            else{
                if(packet.type == ELECTION){
                    if(packet.cmd == ID){    //ELEITO

                        primary = 1;
                        printf("NOVO PRIMARIO 2\n");

                        deleteElement(serverlist, *mainaddr);
                        deleteElement(electlist, servaddr);
                        eleNum--;

                        curPort = send_cli(uList, socksd, curPort);

                    }
                    if(packet.cmd < ID && vote == 0)     // MAIOR QUE O QUE CHEGOU
                        vote = makeElection(electlist,servaddr,ID,socksd, eleNum);
                    if(packet.cmd > ID)     // MENOR QUE O QUE CHEGOU
                        vote = makeElection(electlist,servaddr,packet.cmd,socksd, eleNum);
                }

                //if(packet.type == ELECTED);

                if(packet.type == ALIVE){
                    memcpy(&lists, packet._payload, sizeof(lists));                 // RECEBE AS LISTAS AQUI

                    memcpy(serverlist,lists.slist, sizeof(lists.slist));
                    memcpy(electlist,lists.elist, sizeof(lists.elist));
                    memcpy(uList,lists.uList, sizeof(lists.uList));
                    eleNum = lists.eNum;


                }
            }
        }
    }
}
