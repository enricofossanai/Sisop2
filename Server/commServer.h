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


#define MAX_PACKET_SIZE       64000
#define MAXLINE        		  64000
#define MAXNUMCON   		  100
#define PORT	   		      8000

#define DATA		0
#define ACK			1
#define CMD			2
#define CN          3

typedef struct packet{
    uint16_t type;                  //Tipo do pacote(p.ex. DATA| ACK | CMD)
    uint16_t seqn;                  //Número de sequência
    uint16_t length;                //Comprimento do payload
    uint32_t total_size;            //Número total de fragmentos
    uint32_t checksum;              //checksum
    char _payload[62000];             //Dados do pacote
    } packet;

typedef struct user{
    char username [100];
    struct sockaddr_in cliaddr;
} user;

int checkSum(packet * packet);

void *cliThread(void *arg);

void *sender(void *arg);

void *receiver(void *arg);
