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
#define MAXLINE        		64000
#define MAXNUMCON   		100
#define PORT 4000

#define DATA			0
#define ACK			1
#define CMD			2

typedef struct packet{
    uint16_t type;                    //Tipo do pacote(p.ex. DATA| ACK | CMD)
    uint16_t seqn;                    //Número de sequência
    uint16_t length;                  //Comprimento do payload
    uint32_t total_size;              //Número total de fragmentos
    uint32_t checksum;                //Checksum
    char _payload[62000];             //Dados do pacote
    } packet;
//node of a linked list, being used to store the packege seq that arrived
typedef struct node{
        int data;     //first node is number of seqAck than we have and others are acks recived seqs
        node *next;
     } node;

char * fileBuffer;

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

int main(int argc, char *argv[])
{
	int sockfd, n;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	char buf[256];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("ERROR opening socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0)
		printf("ERROR on binding");

	clilen = sizeof(struct sockaddr_in);

  packet packetBuffer;
//estamos recebendo em um buffer uma mensagem
  int rec;
  while(rec){
    n = recvfrom(sockfd, reinterpret_cast<void *> (&packetBuffer), MAXLINE, MSG_WAITALL, (struct sockaddr *) &cli_addr, &clilen);
    if (n < 0)
      printf("ERROR on recvfrom");
    if (checkSum(packetBuffer) == packetBuffer.checksum){
      if (packetBuffer.type == CMD){
        //TREAT COMMAND
      }
      //check if file is corrupted
      else if(packetBuffer.type == CMD){
        //alocate a buffer for the file
        fileBuffer = (char*)malloc((packetBuffer.length) * sizeof(char));
          if(packetBuffer.total_size == 1){
            //acabou o arquivo ja
            return 0;
          }
          int numSeq = 0;
          //while ()
      }

    }
  }


int numSeqs,totalSeqs;

	while (numSeqs < totalSeqs)) {
		/* receive from socket */
		n = recvfrom(sockfd, buf, 256, 0, (struct sockaddr *) &cli_addr, &clilen);
		if (n < 0)
			printf("ERROR on recvfrom");
		printf("Received a datagram: %s\n", buf);

		/* send to socket */
    //FAZER AQI A ACK
		n = sendto(sockfd, "Got your message\n", 17, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
		if (n  < 0)
			printf("ERROR on sendto");
	}

	close(sockfd);
  free(fileBuffer);
	return 0;
}
