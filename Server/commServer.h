#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_PACKET_SIZE     	64000
#define MAX_PAYLOAD_SIZE        62000
#define MAXNUMCON               100
#define MAX_FILE_NAME_SIZE        100
#define PORT  			        8000
#define TRUE                    1
#define FALSE                   0

#define DATA		0
#define ACK			1
#define CMD			2
#define CN          3

#define CREATE		             0
#define DELETE		             1
#define MODIFY		             2
#define LIST_SERVER        3
#define LIST_CLIENT        4
#define GET_SYNC_DIR       5



//LEMBRAR DE MUDAR LENGTH PARA LONG
typedef struct packet{
    uint16_t type;                              //Tipo do pacote(p.ex. DATA| ACK | CMD)
    uint16_t cmd;                               //Comandos Possiveis \ UPLOAD | DOWNLOAD | DELETE | ...
    uint16_t seqn;                              //Número de sequência
    uint32_t length;                            //Comprimento do payload
    uint32_t total_size;                        //Número total de fragmentos
    uint32_t checksum;                          //Checksum
    char _payload[MAX_PAYLOAD_SIZE];             //Dados do pacote
    } packet;

typedef struct user{
    char username [100];
    struct sockaddr_in cliaddr;
    int socket;
} user;

typedef struct cmdAndFile{
    int command;
    char fileName[MAX_FILE_NAME_SIZE];
  }cmdAndFile;

int checkSum(packet * packet);

int makeSum(packet * packet);

int receiveFile(char *fileName , long int fileSize,  struct sockaddr_in addr, int sockfd);

void *cliThread(void *arg);

int createSocket(user client, int port);

int sendFile(char *fileName, struct sockaddr_in addr, int sockfd);

long int sizeFile (FILE *f);

void *sender(void *arg);


//send a command and wait for and ack
//asks for the name of the file to modify, the server address, the socketfd and the command.
void send_cmd(char *fileName, struct sockaddr_in addr, int sockfd, int command);

//revieves a command and return a structure countaining the command and the name of the file to modify
cmdAndFile rcv_cmd(struct sockaddr_in addr, int sockfd);
