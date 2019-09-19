#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <time.h>

#include "communication.h"

void marshallPacket(packet* inPacket, char* serialized) {
    uint16_t* buffer16 = (uint16_t*) serialized;
    uint32_t* buffer32;
    char* buffer;
    int i = 0;

    printf("Antes do Marshall");
    fflush( stdout );

    *buffer16 = htons(inPacket->type);
    buffer16++;
    printf("Antes do Marshall22");
    fflush( stdout );
    *buffer16 = htons(inPacket->seqn);
    buffer16++;
    *buffer16 = htons(inPacket->length);
    buffer16++;
    buffer32 = (uint32_t*) buffer16;
    *buffer32 = htonl(inPacket->total_size);
    buffer32++;
    *buffer32 = htonl(inPacket->checksum);
    buffer32++;
    buffer = (char*)buffer32;

    for(i = 0; i < MAX_PAYLOAD_SIZE; i++) {
        *buffer = inPacket->_payload[i];
        buffer++;
    }

    return;
}

void unmarshallPacket(packet* outPacket, char* serialized) {
    uint16_t* buffer16 = (uint16_t*) serialized;
    uint32_t* buffer32;
    char* buffer;
    int i = 0;

    outPacket->type = ntohs(*buffer16);
    buffer16++;
    outPacket->seqn = ntohs(*buffer16);
    buffer16++;
    outPacket->length = ntohs(*buffer16);
    buffer16++;
    buffer32 = (uint32_t*)buffer16;
    outPacket->total_size = ntohl(*buffer32);
    buffer32++;
    outPacket->checksum = ntohl(*buffer32);
    buffer32++;
    buffer = (char*)buffer32;


    for(i = 0; i < MAX_PAYLOAD_SIZE; i++) {
        outPacket->_payload[i] = *buffer;
        buffer++;
    }


    return;
}