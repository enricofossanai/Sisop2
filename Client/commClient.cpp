#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <time.h>
#include <math.h>
#include <dirent.h>


#include "commClient.h"

int makeSum(packet * packet) //faz a soma dos dados do pacote
{
    int Sum = 0,Sumchar=0,i;
    int type=packet->type,seqn=packet->seqn,lenght=packet->type,total_size=packet->total_size;

    for(i=0;i<strlen(packet->_payload);i++)
    {
        Sumchar = Sumchar + packet->_payload[i];

    }

    Sum = type + seqn + lenght + total_size + Sumchar;

    return Sum;
}

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

struct sockaddr_in firstConnect (int sockfd , struct hostent *server, char * username){
	struct sockaddr_in servaddr;
	int i;
    char buffer[MAX_PACKET_SIZE];
	socklen_t len = sizeof(struct sockaddr_in);

	memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr = *((struct in_addr *)server->h_addr);

    // Filling packet for connect
    packet sentPacket, recPacket;
    sentPacket.type = CN;
    sentPacket.cmd = 0;
    sentPacket.seqn = 0;
    sentPacket.length = 0;
    sentPacket.total_size = 0;
    strcpy(sentPacket._payload, username);
    sentPacket.checksum = makeSum(&sentPacket);

    i = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr));
	if (i  < 0)
        perror("sendto");

    i = recvfrom(sockfd, reinterpret_cast<void *> (&recPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &servaddr, &len);
    if (i  < 0)
        perror("recvfrom");
    else
        printf("Recebido : %d\n", recPacket.type);


	  fflush( stdout );
/////////////////USANDO ESSA MERDA DE AREA PRA TESTAR
  	//i = sendFile("revistaJuca.txt" , servaddr, sockfd);
////////////////////////////////////
    printf("TO MANDANDO VER\n");

    return servaddr;
}

long int sizeFile (FILE *f){
	long int size;

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


int sendFile(char *fileName , struct sockaddr_in addr, int sockfd){

    FILE *fd = fopen( "revistaJuca.txt", "rb" );
    long int fileSize = sizeFile(fd);
    char *fileBuffer = (char *)malloc((fileSize * 8 * sizeof(char)) + 1);


    if (fd == NULL){
        printf("Erro na abertura do arquivo");
        fclose(fd);
        return -1;
    }

    else{

    socklen_t len = sizeof(struct sockaddr_in);

    //Read file contents into buffer
    size_t paulo = fread(fileBuffer, 1, fileSize, fd);
    if(paulo != fileSize) {
        fprintf(stderr, "Erro ao tentar ler o arquivo inteiro.\n");
        return -1;
    }

	int numSeqs = (fileSize/MAX_PAYLOAD_SIZE);
    int n = 0;
	int curSeq = 0;
	int curAck = 0;
	packet sentPacket, rcvdPacket;
    long int placeinBuffer = 0;
    long int bitstoSend = fileSize;
    sentPacket.cmd = fileSize;
    sentPacket.length = fileSize;

    printf("\nTOTAL DE SEQUENCIAS PARA ENVIAR: %d",numSeqs);
    printf("\nTAMANHO DO ARQUIVO: %ld\n\n",fileSize);
    fflush(stdout);

    n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, MSG_CONFIRM, (const struct sockaddr *) &addr,  sizeof(addr));
    if (n  < 0)
        perror("sendto");

    //while still have packages to send
	while (curSeq <= numSeqs){

        printf("\nENVIANDO NUMERO DE SEQUENCIA: %d\n\n",curSeq);
        fflush(stdout);

		if (fileSize > MAX_PAYLOAD_SIZE)
    		bitstoSend = MAX_PAYLOAD_SIZE;
		else
			bitstoSend = fileSize;

		//while didnt recieved the ack from the package
		while (curAck == curSeq){
	   		sentPacket.type = DATA;
			sentPacket.seqn = curSeq;
			sentPacket.total_size = 0;

            memcpy(sentPacket._payload, &fileBuffer[placeinBuffer], bitstoSend);
            sentPacket.checksum = makeSum(&sentPacket);

			n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &addr,  sizeof(addr));
			if (n  < 0)
        		perror("sendto");

            //n = recv(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0);
			//if(rcvdPacket.seqn == curAck && n >= 0 && (checkSum(&rcvdPacket))){
		    	curAck++;
                printf("Passei aqui cacete\n");
            //}
		}

		placeinBuffer = placeinBuffer + bitstoSend; //move the place of the flag in the buffer for nexzt packet
		fileSize = fileSize - bitstoSend;
		curSeq++;
	}
  }

  printf("COMO PODE SENHOR\n");

 free(fileBuffer);
 fclose(fd);

 return 0;

}

int list_client(char *dirName){

    DIR *dir;
    struct dirent *dent;
    dir = opendir((const char *) dirName);

    if(dir!=NULL)
    {
        while((dent=readdir(dir))!=NULL){
            struct stat info;
            stat(dent->d_name, &info);

            printf("Arquivo:%s\n-Modification Time: %.12s\n-Access Time:%.12s\n-Creation Time:%.12s \n",dent->d_name,4+ctime(&info.st_mtime),4+ctime(&info.st_atime),4+ctime(&info.st_ctime));
        }


        return 1;
    }
    else{
        printf("Erro na abertura do diretório\n");
        return -1;
        }
    fflush(stdout);
    closedir(dir);


}


//sends message to delete file
void send_cmd(char *fileName, struct sockaddr_in addr, int sockfd, int command){
  //filling packet info
    socklen_t len = sizeof(struct sockaddr_in);
    packet sentPacket, rcvdPacket;
    sentPacket.type = CMD;
    sentPacket.cmd = command;
    int n;
    //struct timeval timeout={2,0}; //set timeout for 2 seconds
    //setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
    //sending packet
    int ack = 0;
    while (ack == 0){
        n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &addr,  sizeof(addr));
        if (n  < 0)
            perror("sendto");

        n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &addr, &len);
        if (rcvdPacket.checksum == makeSum(&rcvdPacket)){
            ack = 1;
            printf("\nserver recieved delete command\n");
        }

    }
    //struct timeval timeout={0,0}; //set timeout to return to block
    //setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
    return;

}
