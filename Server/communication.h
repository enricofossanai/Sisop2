#include <stdint.h>


#define MAX_PACKET_SIZE     64000

typedef struct packet{
    uint16_t type;                  //Tipo do pacote(p.ex. DATA| ACK | CMD)
    uint16_t seqn;                  //Número de sequência
    uint16_t length;                //Comprimento do payload
    uint32_t total_size;            //Número total de fragmentos
    uint32_t checksum;              //checksum
    char _payload[62000];             //Dados do pacote
    } packet;

<<<<<<< HEAD

=======
>>>>>>> d36a1feb5d60732dca4cc9b1034048fa0e1a2848
int checkSum(packet * packet);
