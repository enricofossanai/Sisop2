#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>


#define MAX_PACKET_SIZE     	62020
#define MAX_PAYLOAD_SIZE        62000
#define MAX_FILE_NAME_SIZE        100
#define PORT  			        8000
#define TRUE 1
#define FALSE 0

#define DATA		0
#define ACK			1
#define CMD		    2
#define CN          3               // Connect

//Commands
#define CREATE		             0
#define DELETE		             1
#define MODIFY		             2
#define LIST_SERVER              3
#define LIST_CLIENT              4
#define GET_SYNC_DIR             5




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


typedef struct socketInfo{
    int sockfd;
    struct sockaddr_in servaddr;
    socklen_t len;
    } socketInfo;

typedef struct cmdAndFile{
    int command;
    char fileName[MAX_FILE_NAME_SIZE];
  }cmdAndFile;

int makeSum(packet * packet);

int checkSum(packet * packet);

//thread that communicate with the server
void *clientComm(void *arg);

//thread to handle changes in sync_dir
void *clientNotify(void *arg);

//send first message to server to connect
struct sockaddr_in firstConnect (int sockfd , struct hostent *server, char * username);

//return size of file or -1 if error
long int sizeFile (FILE *f);

//copies file to buffer
long fileToBuffer (FILE *f);

//Send file
int sendFile(char *fileName, struct sockaddr_in addr, int sockfd);

//Self explained
int receiveFile(char *fileName , long int fileSize,  struct sockaddr_in addr, int sockfd);

//list files from user sync_dir
int list_client(char * dirName);

//send a command and wait for and ack
//asks for the name of the file to modify, the server address, the socketfd and the command.
void send_cmd(char *fileName, struct sockaddr_in addr, int sockfd, int command);

//revieves a command and return a structure countaining the command and the name of the file to modify
cmdAndFile rcv_cmd(struct sockaddr_in addr, int sockfd);
