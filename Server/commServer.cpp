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
#include <math.h>

#include "commServer.h"

using namespace std;

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

void addToONlist (userList **list, user *con){
  if (list!=NULL){
    userList *newConnection = (userList*)malloc(sizeof(userList));
    newConnection->connection = *con;
    newConnection->next = (*list)->next;
    (*list)->next = newConnection;
    return;
  }
}

void rmvFromONlist (userList **list, user *usr){
  int deleted = 0;
  userList* temp1 = NULL;
  userList* temp2 = NULL;
  userList* traverse = *list;   // *** make a copy that we can use to traverse the list
  while (deleted == 0)
  {
      if (traverse->next->connection.socket == usr->socket)//check in node ahead
      {
          temp1 = traverse;                           // *** Use local copy of pointer
          temp2 = traverse->next;//the one to free    // *** Use local copy of pointer
          temp1->next = temp2->next;
          free(temp2);
          deleted = 1;
          return;
      }
      traverse = traverse->next;                         // *** Use local copy of pointer
  }
  return;
}

void displayList(userList* head){
  userList *temp;
  if(head == NULL){
    printf("List is empty.");
    fflush( stdout );
  }
  else{
    temp = head;
    temp = temp->next;
    printf("\nONLINE USERS LIST:");
    while(temp != NULL){
      printf("\nUser = %s \nIP: %d", (temp->connection).username, (temp->connection).socket); // Print data of current node
      fflush( stdout );
      temp = temp->next;                 // Move to next node
    }
    printf("\n");
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

    //Create dir for client
    DIR* dir = opendir(client.username);
    if(dir){
        printf("Directory already exists\n");
        closedir(dir);
    }
    else{
        mkdir(client.username,0777);
        printf("Directory created\n");
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

int receiveFile(char *fileName , long int fileSize,  struct sockaddr_in addr, int sockfd){
    FILE *fd = fopen( fileName , "wb" );
    unsigned char *bufferFile = (unsigned char *)malloc(fileSize);

    if (fd == NULL){
        printf("Deu pau no arquivo\n");
        return -1;
    }

    else{
        socklen_t len = sizeof(struct sockaddr_in);

        int numSeqs = (fileSize/MAX_PAYLOAD_SIZE);
        int n;
        int curSeq = 0;
        int *allSeq = (int *)malloc((numSeqs * sizeof(int)) + 1);
        packet sentPacket, rcvdPacket;
        long bitstoReceive = 0;
        long int toWrite = fileSize;

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
                sentPacket.seqn = rcvdPacket.seqn;
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

        fclose(fd);
        free(allSeq);
        free(bufferFile);

        return 0;
    }
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

        sentPacket.type = CMD;
        sentPacket.cmd = 0;
        sentPacket.seqn = 0;
        sentPacket.length = fileSize;
        sentPacket.total_size = 0;
        strcpy(sentPacket._payload, "");
        sentPacket.checksum = makeSum(&sentPacket);

        n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, MSG_CONFIRM, (const struct sockaddr *) &addr,  sizeof(addr));
        if (n  < 0)
            perror("sendto");

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
    			sentPacket.total_size = 0;

                memcpy(sentPacket._payload, fileBuffer + placeinBuffer, bitstoSend);
                sentPacket.checksum = makeSum(&sentPacket);

    			n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, MSG_CONFIRM, (struct sockaddr *) &addr,  sizeof(addr));
    			if (n  < 0)
            		perror("sendto");

                n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PAYLOAD_SIZE, 0, NULL, NULL);      // TEM QUE SER PAYLOAD PQ DEUS QUER
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

//sends message to create, delete or modify file
void send_cmd(char *fileName, struct sockaddr_in addr, int sockfd, int command){
  //filling packet info
    socklen_t len = sizeof(struct sockaddr_in);
    packet sentPacket, rcvdPacket;
    sentPacket.type = CMD;
    sentPacket.cmd = command;
    strcpy(sentPacket._payload,fileName);
    sentPacket.checksum = makeSum(&sentPacket);

    int n;
    //struct timeval timeout={2,0}; //set timeout for 2 seconds
    //setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
    //sending packet
    int ack = 0;
    while (ack == 0){
            //printf("Enviando mensagem\n") ;
        n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &addr,  sizeof(addr));
        if (n  < 0)
            perror("sendto");
            //printf("Esperando Ack\n") ;
        n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &addr, &len);
        if (n  < 0)
            perror("recvfrom");
        if (rcvdPacket.checksum == makeSum(&rcvdPacket)){
            ack = 1;
            printf("\nclient recieved command\n");
        }
        printf("Tentando enviar mensagem novamente\n") ;
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
    fflush(stdout);
    n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &addr, &len);
    if (n  < 0)
        perror("recvfrom");
    if (rcvdPacket.checksum == makeSum(&rcvdPacket)){
        //printf("\nserver recieved command %d\n", rcvdPacket.cmd);
        sentPacket.type = ACK;
        sentPacket.cmd = rcvdPacket.cmd;
        sentPacket.checksum = makeSum(&sentPacket);
        n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &addr,  sizeof(addr));
        fflush(stdout);
        if (n  < 0)
              perror("sendto");
        returnFile.command = rcvdPacket.cmd;
        strcpy(returnFile.fileName, rcvdPacket._payload);
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
