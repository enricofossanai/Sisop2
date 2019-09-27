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
extern struct sockaddr_in servaddr;
extern int sockfd;
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
  sendFile("oitenta");
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
long fileToBuffer (FILE *f){
  long size = sizeFile(f);
  fileBuffer = (char*)malloc((size) * sizeof(char));

  //Read file contents into buffer
  size_t paulo = fread(fileBuffer, 1, size, f);
  if(paulo != size) {
      fprintf(stderr, "Erro ao tentar ler o arquivo inteiro.\n");
      return -1;
  }
  printf("%s",fileBuffer);
  return size;
}

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



int sendFile(char *fileName){
  FILE *fd = fopen( "testfile.txt", "rb" );
  if (fd!=NULL){

    long fileSize = fileToBuffer(fd);

    printf("\nsizeofbuffer:%ld\n",fileSize);

    if (fileSize <= MAX_PAYLOAD_SIZE){
      printf("\nonly Sending one Package\n");
      //send single package
    }
    else{
      printf("\nsending multiple packages\n");

      //creating and initializing list o seqAcks
      node *list = (node*)malloc(sizeof(node));
      list->data = 0;

      //while stick acks missing keeps retransmiting
      /*while (numbeOfAcks < numberOfSequences){

      }
      */
      //deleting the list
      deleteList(list);
      free(list);
      list = NULL;
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
