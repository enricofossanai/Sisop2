#include <stdint.h>


#define MAX_PACKET_SIZE     	64000
#define MAXLINE        		64000
#define MAXNUMCON   		100
#define PORT	   		8000

typedef struct packet{
    uint16_t type;                  //Tipo do pacote(p.ex. DATA| ACK | CMD)
    uint16_t seqn;                  //Número de sequência
    uint16_t length;                //Comprimento do payload
    uint32_t total_size;            //Número total de fragmentos
    uint32_t checksum;              //checksum
    char _payload[62000];             //Dados do pacote
    } packet;

int checkSum(packet * packet);

void *sender(void *arg);

void *receiver(void *arg);
