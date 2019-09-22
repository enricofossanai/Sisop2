#include <stdint.h>


#define MAX_PAYLOAD_SIZE 1024

typedef struct packet{
    uint16_t type;                                          //Tipo do pacote(p.ex. DATA| ACK | CMD)
    uint16_t seqn;                                  //Número de sequência
    uint16_t length;                                //Comprimento do payload
    uint32_t total_size;                                //Número total de fragmentos
    uint32_t checksum;                                  //checksum
    char _payload[100];               //Dados do pacote
    } packet;



int checkSum(packet * packet);