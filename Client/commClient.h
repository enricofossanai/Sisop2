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


#define MAX_PACKET_SIZE     	64000
#define MAX_PAYLOAD_SIZE        62000
#define PORT  			8000
#define TRUE 1
#define FALSE 0

#define DATA		0
#define ACK			1
#define CMD		2
#define CN          3               // Connect

#define UPLOAD		0
#define DOWNLOAD	1
#define DELETE		2


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

//node of a linked list, being used to store the packege seq that arrived
typedef struct node{
        int data;
        node *next;
     } node;

int checkSum(packet * packet);

//thread that communicate with the server
void *clientComm(void *arg);

//thread to handle changes in sync_dir
void *clientNotify(void *arg);

//send first message to server to connect
struct sockaddr_in firstConnect (int sockfd , struct hostent *server, char * username);

//return size of file or -1 if error
long sizeFile (FILE *f);

//copies file to buffer
long fileToBuffer (FILE *f);

//sends file
int sendFile(char *fileName, struct sockaddr_in addr, int sockfd);
