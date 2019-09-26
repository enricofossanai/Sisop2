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

#include "commClient.h"

//chamando as variabeis globais
extern char * fileBuffer;

int checkSum(packet * packet) //faz a soma dos dados do pacote
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

void firstConnect (int sockfd , struct hostent *server){
	struct sockaddr_in servaddr;
	int i;
    char buffer[MAX_PACKET_SIZE];
	socklen_t len = sizeof(servaddr);

	memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr = *((struct in_addr *)server->h_addr);


    // Filling packet for test
    packet sentPacket;
    sentPacket.type = 2;
    sentPacket.seqn = 0;
    sentPacket.length = 0;
    sentPacket.total_size = 0;
    strcpy(sentPacket._payload, "");
    sentPacket.checksum = checkSum(&sentPacket);

    i = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr));
	if (i  < 0)
        printf("\nERROR on sendto First Connect\n");
   	else
		printf("Connection Made\n");

	fflush( stdout );



/////////////////USANDO ESSA MERDA DE AREA PRA TESTAR
//  printf("%s",fileBuffer);
//  fflush( stdout );
////////////////////////////////////


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

//copies file to buffer
int fileToBuffer (FILE *f){
  long size = sizeFile(f);
  fileBuffer = (char*)malloc((size) * sizeof(char));

  //Read file contents into buffer
  size_t paulo = fread(fileBuffer, 1, size, f);
  if(paulo != size) {
      fprintf(stderr, "Erro ao tentar ler o arquivo inteiro.\n");
      return -1;
  }
  printf("%s",fileBuffer);
  free(fileBuffer);
  return 0;
}

int sendFile(char *fileName){
  FILE *fd;
  fd = fopen("myfile", "rb");

  //fclose(fd);
  return 0;
}
