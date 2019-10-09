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
        printf("Conectado com  Servidor");


	  fflush( stdout );

      return servaddr;
}

int connectListener (int sockfd , struct sockaddr_in servaddr, char * username){
	int i;
    struct sockaddr_in cliaddr;
	socklen_t len = sizeof(struct sockaddr_in);
    srand(time(NULL));

    // Filling server information
    cliaddr.sin_family    = AF_INET; // IPv4
    cliaddr.sin_addr.s_addr = INADDR_ANY;
    cliaddr.sin_port = htons(9000 + (rand() % 15));                 // Valor arbitrario longe de 8000

    struct timeval timeout={2,0}; //set timeout for 2 seconds
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0 )
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Filling packet for connect
    packet sentPacket, recPacket;
    sentPacket.type = CNL;
    sentPacket.cmd = 0;
    sentPacket.seqn = 0;
    sentPacket.length = 0;
    sentPacket.total_size = 0;
    strcpy(sentPacket._payload, username);
    sentPacket.checksum = makeSum(&sentPacket);

    recPacket.type = 10;

    while(recPacket.type != ACK){

        i = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr));
    	if (i  < 0)
            perror("sendto");

        i = recvfrom(sockfd, reinterpret_cast<void *> (&recPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &servaddr, &len);
        if (i  > 0 && recPacket.type == ACK)
            printf("Conectado Listener Socket com Server");

    }
	  fflush( stdout );

      struct timeval notimeout={0,0}; //set timeout for 2 seconds
      setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&notimeout,sizeof(struct timeval));

      return sockfd;
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

int sendFile(char *fileName, struct sockaddr_in addr, int sockfd){

    FILE *fd = fopen( fileName, "rb" );
    long int fileSize = sizeFile(fd);

    int numSeqs = (fileSize/MAX_PAYLOAD_SIZE);
    int n = 0;
    int curSeq = 0;
    int curAck = 0;
    packet sentPacket, rcvdPacket;
    long int placeinBuffer = 0;
    long int bitstoSend = fileSize;
    socklen_t len = sizeof(struct sockaddr_in);
    size_t paulo;
    unsigned char *fileBuffer = (unsigned char *)malloc(fileSize);

    if (fd == NULL){
        printf("Erro no arquivo");
        fclose(fd);
        return -1;
    }

    else{
        //Read file contents into buffer
        paulo = fread(fileBuffer, 1, fileSize, fd);
        if(paulo != fileSize) {
            printf("Erro ao tentar ler o arquivo inteiro.\n");
            return -1;
        }


        //while still have packages to send
    	while (curSeq <= numSeqs){

    		if (fileSize > MAX_PAYLOAD_SIZE)
        		bitstoSend = MAX_PAYLOAD_SIZE;
    		else
    			bitstoSend = fileSize;

    		//while didnt received the ack from the package
    		while (curAck == curSeq){
    	   		sentPacket.type = DATA;
    			sentPacket.seqn = curSeq;
    			sentPacket.total_size = 0;

                memcpy(sentPacket._payload, fileBuffer + placeinBuffer, bitstoSend);
                sentPacket.checksum = makeSum(&sentPacket);

    			n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, MSG_CONFIRM, (struct sockaddr *) &addr,  sizeof(addr));
    			if (n  < 0)
            		perror("sendto");

                n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, NULL, NULL);      // TEM QUE SER PAYLOAD PQ DEUS QUER
                if (rcvdPacket.seqn == curAck && n > 0){
    		    	    curAck++;
                }
    		}

    		placeinBuffer = placeinBuffer + bitstoSend; //move the place of the flag in the buffer for next packet
    		fileSize = fileSize - bitstoSend;
    		curSeq++;
    	}
  }

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


//sends message to create, delete or modify file
void send_cmd(char *fileName, struct sockaddr_in addr, int sockfd, int command, char *dir){
  //filling packet info
    socklen_t len = sizeof(struct sockaddr_in);
    packet sentPacket, rcvdPacket;

    memset(&sentPacket, 0 , sizeof(struct packet));
    memset(&rcvdPacket, 0 , sizeof(struct packet));

    sentPacket.type = CMD;
    sentPacket.cmd = command;
    strcpy(sentPacket._payload,fileName);


    int n;
    //struct timeval timeout={2,0}; //set timeout for 2 seconds
    //setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

    //sending packet
    int ack = 0;

    if (command == CREATE || command == MODIFY){
        sleep(2);
        FILE *fd = fopen( dir, "rb" );
        if(fd == NULL)
            printf("Erro no arquivo\n");

        sentPacket.length = sizeFile(fd);
        fclose(fd);
    }

    sentPacket.checksum = makeSum(&sentPacket);

    while (ack == 0){
            //printf("Enviando mensagem\n") ;
        n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &addr,  sizeof(addr));
        if (n  < 0)
            perror("sendto");
            //printf("Esperando Ack\n") ;
        n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &addr, &len);
        if (n  < 0)
            perror("recvfrom");
        if (checkSum(&rcvdPacket)){
            ack = 1;
            printf("\nserver received command\n");
        }
    }
    //struct timeval timeout={0,0}; //set timeout to return to block
    //setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
    return;
}

cmdAndFile rcv_cmd(struct sockaddr_in addr, int sockfd){
  //filling packet info
    socklen_t len = sizeof(struct sockaddr_in);
    packet sentPacket, rcvdPacket;
    int n;
    cmdAndFile returnFile;
    returnFile.command = -1;
    //printf("\nEsperando Comando...");
    //fflush(stdout);
    n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &addr, &len);
    if (n  < 0)
        perror("recvfrom");
    if (checkSum(&rcvdPacket)){
        //printf("\nserver received command %d\n", rcvdPacket.cmd);
        sentPacket.type = ACK;
        sentPacket.cmd = rcvdPacket.cmd;
        sentPacket.checksum = makeSum(&sentPacket);

        n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &addr,  sizeof(addr));
        fflush(stdout);
        if (n  < 0)
            perror("sendto");

        returnFile.command = rcvdPacket.cmd;
        strcpy(returnFile.fileName, rcvdPacket._payload);
        returnFile.fileSize = rcvdPacket.length;

        return returnFile;
      } else{
            printf("\nERRO DE CHECKSUM NO  COMANDO");
            fflush(stdout);
          }
    //struct timeval timeout={0,0}; //set timeout to return to block
    //setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
    printf("\nSAIU DO RCV_CMD");
    return returnFile;
}

int receiveFile(char *fileName , long int fileSize,  struct sockaddr_in addr, int sockfd){
    FILE *fd = fopen( fileName , "wb" );
    unsigned char *bufferFile = (unsigned char *)malloc(fileSize + 1);

    if (fd == NULL){
        printf("Deu pau no arquivo\n");
        return -1;
    }

    else{

        struct timeval timeout={0,0}; //set timeout to return to block
        setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

        socklen_t len = sizeof(struct sockaddr_in);

        int numSeqs = (fileSize/MAX_PAYLOAD_SIZE);
        int n;
        int curSeq = 0;
        int *allSeq = (int *)malloc(sizeof(int) * (numSeqs + 1));
        packet sentPacket, rcvdPacket;
        long bitstoReceive = 0;
        long int toWrite = fileSize;

        memset(allSeq, 0, sizeof(int) * (numSeqs + 1));


        //while still have packages to receive
        while (curSeq <= numSeqs){

            if (fileSize > MAX_PAYLOAD_SIZE)
                bitstoReceive = MAX_PAYLOAD_SIZE;
            else
                bitstoReceive = fileSize;

            n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &addr, &len);
            if(n < 0)
                perror("recvfrom");

            if((checkSum(&rcvdPacket))){		                                    // Verificação de CheckSum

                memcpy((bufferFile + (MAX_PAYLOAD_SIZE * rcvdPacket.seqn)), rcvdPacket._payload, bitstoReceive);

                sentPacket.type = ACK;
                sentPacket.seqn = curSeq;
                sentPacket.length = 0;
                sentPacket.total_size = 0;
                strcpy(sentPacket._payload, "");
                sentPacket.checksum = makeSum(&sentPacket);

                n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &addr,  sizeof(addr));
                if (n  < 0)
                    perror("sendto");

                if (allSeq[rcvdPacket.seqn] == 0){
                    fileSize = fileSize - bitstoReceive;
                    curSeq++;
                    allSeq[rcvdPacket.seqn] = 1;
                }
            }
        }

        //closes file and free the buffer
        size_t jubileu = fwrite(bufferFile, 1, toWrite, fd);
        if(jubileu != toWrite) {
            printf("Erro ao tentar escrever o arquivo inteiro.\n");
            return -1;
        }

        struct timeval notimeout={0,0}; //set timeout to return to block
        setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&notimeout,sizeof(struct timeval));

        fclose(fd);
        free(allSeq);
        free(bufferFile);


        return 0;
    }
}

void copyFile(char *source_file, char *target_file){


    long int fileSize;
    FILE *source, *target;

   source = fopen(source_file, "rb");

   if (source == NULL)
   {
       printf("Problema no arquivo\n");
      return;
   }
   fileSize = sizeFile(source);
   unsigned char *ch = (unsigned char *)malloc(fileSize);

   target = fopen(target_file, "wb");

   if (target == NULL)
   {
      fclose(source);
      printf("Problema no arquivo\n");
      return;
   }

   fread(ch, 1 , fileSize,source);
   fwrite(ch , 1 ,fileSize, target);


   printf("File copied successfully.\n");

   fclose(source);
   fclose(target);

   return;

}
