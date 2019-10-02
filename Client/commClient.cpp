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
extern int fileParts;

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
    sentPacket.checksum = checkSum(&sentPacket);

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
  	sendFile("oitenta" , servaddr, sockfd);
////////////////////////////////////
    return servaddr;
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



/*
void ackSequence (node **list, int seqn){
  if (list!=NULL){
    node *newNode = (node*)malloc(sizeof(node));
    newNode->data = seqn;
    newNode->next = (*list)->next;
    (*list)->next = newNode;
    (*list)->data++;

    printf("ListSize = %d\n", (*list)->data);
    fflush( stdout );
    printf("FirstNode = %d\n", (*list)->next->data);
    fflush( stdout );
    return;
  }
}

void deleteList(node* head)  {

  node *current = head;
  node *next;
  while (current != NULL){
    next = current->next;
    free(current);
    current = next;
  }
  head = NULL;
}

void displayList(node* head)
{
  node *temp;
  if(head == NULL){
    printf("List is empty.");
    fflush( stdout );
  }
  else{
    temp = head;
    printf("\n");
    while(temp != NULL){
      printf("Data = %d ", temp->data); // Print data of current node
      fflush( stdout );
      temp = temp->next;                 // Move to next node
    }
    printf("\n");
  }
}

//check if already recieved ack from sent data packages
//returns 1 if yes and 0 if no
int checkSeqAck(){
  return 0;
}
*/


int sendFile(char *fileName , struct sockaddr_in servaddr, int sockfd){
  FILE *fd = fopen( "testFile.txt", "rb" );
  if (fd!=NULL){
    char * fileBuffer;
    long fileSize = sizeFile(fd);
    fileBuffer = (char*)malloc((fileSize) * sizeof(char));

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

	printf("\nsize of buffer:%ld\n",fileSize);
	//while still have packages to send


    printf("NUMERO DE SEQUENCIAS:%d\n", numSeqs);

//while still have packages to send
	while (curSeq <= numSeqs){
    	
		if (fileSize > MAX_PAYLOAD_SIZE){
    		bitstoSend = MAX_PAYLOAD_SIZE;
      	}

		else
			bitstoSend = fileSize;


			//while didnt recieved the ack from the package
		while (curAck == curSeq){
	   		sentPacket.type = DATA;
			sentPacket.seqn = curSeq;
			sentPacket.length = 0;
			sentPacket.total_size = 0;
			
			memcpy((sentPacket._payload), (fileBuffer + placeinBuffer), bitstoSend);


		    //SENDING PACKAGE
		    //n = sendto( , , , , );
		  	//if (n < 0)
		  	//	printf("ERROR sendto");
		    //RECIEVING ACK NEED TIMEOUT
		    //n = rcvfrom( , , , , );
		    //if (n < 0)
		  	//	printf("ERROR recvfrom");

		  	//}

			n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &servaddr,  sizeof(servaddr));
			if (n  < 0)
        		perror("sendto");

			// Timeout? RecvFrom é bloqueante e se o ACK não chegar?

   			//n = recvfrom(sockfd, reinterpret_cast<void *> (&recPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &servaddr, &len);
    		//if (n  < 0)
        	//	perror("recvfrom");
			//if(rcvdPacket.seqn == curAck)
		    	curAck++;

			}
		
		printf("Onde estamos : %d\nNa sequencia : %d\n", placeinBuffer, curSeq);
		placeinBuffer = placeinBuffer + bitstoSend; //move the place of the flag in the buffer for nexzt packet
		fileSize = fileSize - bitstoSend;
		curSeq++;
	}


    //closes file and free the buffer
    free(fileBuffer);
    fclose (fd);
    return 0;
  }
  else
      printf("Erro na abertura do arquivo");
  return -1;
}


int receiveFile(){

}
