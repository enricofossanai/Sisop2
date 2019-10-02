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

#include "commServer.h"


int checkSum(packet * packet) //verifica se o valor da soma dos dados é a mesmo( retorna 1 caso for o mesmo, -1 caso contrario)
{
    int Sum = 0,Sumchar=0,i;
    int type=packet->type,seqn=packet->seqn,lenght=packet->type,total_size=packet->total_size;

    for(i=0;i<strlen(packet->_payload);i++)
    {
        Sumchar = Sumchar + packet->_payload[i];

    }


    Sum = type + seqn + lenght + total_size + Sumchar;

    if (Sum==packet->checksum)
        return 1;
    else
    {
        return -1;
    }

}

int createSocket(user client, int port){
    int sockfd;
    int i,n;
    struct sockaddr_in servaddr;
    packet sendPacket;
    socklen_t len = sizeof(servaddr);


    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));


    printf("Criando Socket do Cliente : %s\n", client.username);

    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    sendPacket.type = ACK;
    sendPacket.cmd = 0;
    sendPacket.seqn = 0;
    sendPacket.length = 0;
    sendPacket.total_size = 0;
    strcpy(sendPacket._payload, "");
    sendPacket.checksum = checkSum(&sendPacket);

    i = sendto(sockfd, reinterpret_cast<void *> (&sendPacket), MAX_PACKET_SIZE, 0,(const struct sockaddr *) &(client.cliaddr), sizeof(struct sockaddr));
    if (i < 0)
        printf("ERROR on sendto\n");
    else
        printf("ACK enviado, Socket Criado : %d\n" , sockfd);

    return(sockfd);
}
