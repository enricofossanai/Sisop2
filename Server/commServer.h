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

#include "fileManager.h"

#define MAX_PACKET_SIZE     	62020
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
#define CNL         4               // Connect Listener
#define CS          5               // Connect Server
#define CE          6               // Connect Election address

#define CREATE		             0
#define DELETE		             1
#define MODIFY		             2
#define LIST_SERVER              3
#define LIST_CLIENT              4
#define GET_SYNC_DIR             5
#define EXIT                     6
#define DOWNLOAD                 7
#define CLIENT                   8
#define NAME                     9
#define SERVER                   10

#define ALIVE                    0
#define ELECTION                 1
#define ELECTED                  2


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
    char username [24];
    struct sockaddr_in cliaddr;
    int socket;
    struct sockaddr_in cliSend;
} user;

typedef struct cmdAndFile{
    int command;
    char fileName[MAX_FILE_NAME_SIZE];
    long int fileSize;
  }cmdAndFile;

typedef struct backupComm{
    struct sockaddr_in slist [10];
    struct sockaddr_in elist [10];
    int eNum;
    user uList[10];
    }backupComm;

typedef struct userList{
    user connection;
    userList *next;
    } userList;


int checkSum(packet * packet);

int makeSum(packet * packet);

int receiveFile(char *fileName , long int fileSize,  struct sockaddr_in addr, int sockfd);

void *cliThread(void *arg);

void *election(void *arg);

void *serverComm(void *arg);

void createDir(char *name);

int createSocket(user client, int port);

struct sockaddr_in getClientLSocket(user client, int socket);

void addToONlist (user *uList, user usr);

void rmvFromONlist (user *uList, user *usr);

void displayList(user* uList);

struct sockaddr_in getUserList(user *uList, user *usr);

int sendFile(char *fileName, struct sockaddr_in addr, int sockfd);

long int sizeFile (FILE *f);

void *sender(void *arg);


//send a command and wait for and ack
//asks for the name of the file to modify, the server address, the socketfd and the command.
void send_cmd(char *payload, struct sockaddr_in addr, int sockfd, int command, char *dir);

//revieves a command and return a structure countaining the command and the name of the file to modify
cmdAndFile rcv_cmd(struct sockaddr_in addr, int sockfd);

void make_cmd (cmdAndFile lastCommand, user *client, char *dirClient, user *uList, struct sockaddr_in serverlist [10], int eleNum);

void server_cmd(cmdAndFile lastCommand,struct sockaddr_in addr , char *user, int sockfd);

char* backup_rcvd (packet rcvdPacket, struct sockaddr_in addr, int sockfd);

void connectBackup (int sockfd , struct hostent *server, int servType);

int makeElection ( struct sockaddr_in electlist[10],struct sockaddr_in servaddr,int ID,int socksd, int eleNum);

void deleteElement(struct sockaddr_in *list, struct sockaddr_in x);

int send_cli(user *uList, int socksd, int port);
