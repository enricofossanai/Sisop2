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

#define MAX_PACKET_SIZE     	64000
#define MAX_PAYLOAD_SIZE      62000
#define MAXLINE        		64000
#define MAXNUMCON   		100
#define PORT 4000


typedef struct packet{
    uint16_t type;                    //Tipo do pacote(p.ex. DATA| ACK | CMD)
    uint16_t seqn;                    //Número de sequência
    uint16_t length;                  //Comprimento do payload
    uint32_t total_size;              //Número total de fragmentos
    uint32_t checksum;                //Checksum
    char _payload[MAX_PAYLOAD_SIZE];             //Dados do pacote
    } packet;
//node of a linked list, being used to store the packege seq that arrived
typedef struct node{
        int data;     //first node is number of seqAck than we have and others are acks recived seqs
        node *next;
     } node;

char * fileBuffer;

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

int main(int argc, char *argv[]){

  //opening socket
  #define MAX_PACKET_SIZE     	64000
  #define MAX_PAYLOAD_SIZE      62000

  int sockfd, n;
  unsigned int length;
  struct sockaddr_in serv_addr, from;
  struct hostent *server;

  if (argc < 2) {
    fprintf(stderr, "usage %s hostname\n", argv[0]);
    exit(0);

  }

  server = gethostbyname(argv[1]);
  if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
  }

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    printf("ERROR opening socket");


  length = sizeof(struct sockaddr_in);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
  bzero(&(serv_addr.sin_zero), 8);



  FILE *fd = fopen( "testfile.txt", "rb" );
    if (fd!=NULL){


      char buffer[MAX_PACKET_SIZE];
      long fileSize = fileToBuffer(fd);

      printf("\nsizeofbuffer:%ld\n",fileSize);

      if (fileSize <= MAX_PAYLOAD_SIZE){
        printf("\nonly Sending one Package\n");
        packet sentPacket;
        //send single package
      	n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, MSG_CONFIRM, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
      	if (n < 0)
      		printf("ERROR sendto");

      	length = sizeof(struct sockaddr_in);
      	n = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) &from, &length);
      	if (n < 0)
      		printf("ERROR recvfrom");

      	printf("Got an ack: %s\n", buffer);
        }

      else{
        printf("\nsending multiple packages\n");

        //creating and initializing list o seqAcks
        node *list = (node*)malloc(sizeof(node));
        list->data = 0;

        //while stick acks missing keeps retransmiting
        int acks,totalSeqs;
        while (acks < totalSeqs){

        }

        //deleting the list
        deleteList(list);
        free(list);
        list = NULL;
      }

      //closes file and free the buffer
      free(fileBuffer);
      close(sockfd);
      fclose (fd);
      return 0;
    }
  }