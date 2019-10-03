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

    sendPacket.type = CMD;
    sendPacket.cmd = 0;
    sendPacket.seqn = 0;
    sendPacket.length = 0;
    sendPacket.total_size = 0;
    strcpy(sendPacket._payload, "");
    sendPacket.checksum = checkSum(&sendPacket);

    n = sendto(sockfd, reinterpret_cast<void *> (&sendPacket), MAX_PACKET_SIZE, 0,( struct sockaddr *) &(client.cliaddr), sizeof(struct sockaddr));
    if (n < 0)
        perror("sendto");

    fflush( stdout );
    return sockfd;
}

long sizeFile (FILE *f){
	long size;

	if (f != NULL) {
    fseek(f, 0, SEEK_END);
    // pega a posição corrente de leitura no arquivo
    size = ftell(f);
		fseek(f,0,SEEK_SET);
		return size;

    } else {
        printf("Arquivo inexistente");
		return -1;
    }

}

int sendFile(char *fileName , struct sockaddr_in addr, int sockfd){             // Lembrar do // nos pathname!!!!!
    FILE *fd = fopen( fileName , "rb" );
    if (fd!=NULL){
    char * fileBuffer;
    long fileSize = sizeFile(fd);
    fileBuffer = (char*)malloc((fileSize) * sizeof(char));
    socklen_t len = sizeof(struct sockaddr_in);

    //Read file contents into buffer
    size_t paulo = fread(fileBuffer, 1, fileSize, fd);
    if(paulo != fileSize) {
        fprintf(stderr, "Erro ao tentar ler o arquivo inteiro.\n");
        return -1;
    }

	int numSeqs = (fileSize/MAX_PAYLOAD_SIZE);
    int n;
	int curSeq = 0;
	int curAck = 0;
	packet sentPacket, rcvdPacket;
    long placeinBuffer = 0;
    long bitstoSend = fileSize;

    //while still have packages to send
	while (curSeq <= numSeqs){

		if (fileSize > MAX_PAYLOAD_SIZE)
    		bitstoSend = MAX_PAYLOAD_SIZE;
		else
			bitstoSend = fileSize;


		//while didnt recieved the ack from the package
		while (curAck == curSeq){
	   		sentPacket.type = DATA;
			sentPacket.seqn = curSeq;
			sentPacket.length = 0;
			sentPacket.total_size = 0;

			memcpy((sentPacket._payload), (fileBuffer + placeinBuffer), bitstoSend);

			n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &addr,  sizeof(addr));
			if (n  < 0)
        		perror("sendto");

            struct timeval timeout={2,0}; //set timeout for 2 seconds
            setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

   			n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &addr, &len);
			if(rcvdPacket.seqn == curAck && n >= 0)
		    	curAck++;

			}
		placeinBuffer = placeinBuffer + bitstoSend; //move the place of the flag in the buffer for nexzt packet
		fileSize = fileSize - bitstoSend;
		curSeq++;
	}

    struct timeval timeout={0,0}; //remove timeout
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
    //closes file and free the buffer
    free(fileBuffer);
    return 0;
  }
  else{
      printf("Erro na abertura do arquivo");
  return -1;
 }
}

int receiveFile(char *fileName , long fileSize, struct sockaddr_in addr, int sockfd){
    FILE *fd = fopen( fileName , "rb" );
    if (fd!=NULL){

    char *fileBuffer;
    fileBuffer = (char*)malloc((fileSize) * sizeof(char));
    socklen_t len = sizeof(struct sockaddr_in);

    int numSeqs = (fileSize/MAX_PAYLOAD_SIZE);
    int n;
    int curSeq = 0;
    int *allSeq = (int*)malloc(sizeof(int) * numSeqs);
    packet sentPacket, rcvdPacket;
    long placeinBuffer = 0;
    long bitstoReceive = fileSize;

    //while still have packages to receive
    while (curSeq <= numSeqs){

        if (fileSize > MAX_PAYLOAD_SIZE)
            bitstoReceive = MAX_PAYLOAD_SIZE;
        else
            bitstoReceive = fileSize;

        n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &addr, &len);
        if(n < 0)
            perror("recvfrom");

        memcpy((fileBuffer + placeinBuffer), (sentPacket._payload + (MAX_PAYLOAD_SIZE * rcvdPacket.seqn)), bitstoReceive);

        sentPacket.type = ACK;
        sentPacket.seqn = rcvdPacket.seqn;
        sentPacket.length = 0;
        sentPacket.total_size = 0;

        n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &addr,  sizeof(addr));
        if (n  < 0)
            perror("sendto");

        if (allSeq[rcvdPacket.seqn] == 0){
        fileSize = fileSize - bitstoReceive;
        curSeq++;
        allSeq[rcvdPacket.seqn] = 1;
        }
    }

    //closes file and free the buffer
    free(fileBuffer);
    return 0;
    }

    else{
        printf("Erro na abertura do arquivo");
    return -1;
    }
}
