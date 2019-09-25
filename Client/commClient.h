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
#define PORT  			8000

#define DATA			0
#define ACK			1
#define CMD			2

typedef struct packet{
    uint16_t type;                    //Tipo do pacote(p.ex. DATA| ACK | CMD)
    uint16_t seqn;                    //Número de sequência
    uint16_t length;                  //Comprimento do payload
    uint32_t total_size;              //Número total de fragmentos
    uint32_t checksum;                //Checksum
    char _payload[62000];             //Dados do pacote
    } packet;


typedef struct socketInfo{
    int sockfd;
    struct sockaddr_in servaddr;
    socklen_t len;
    } socketInfo;


int checkSum(packet * packet);

//thread that sends messages to the server
void *sender(void *arg);

void firstConnect (int sockfd , struct hostent *server);

