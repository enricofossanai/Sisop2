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


//node of a linked list, being used to store the packege seq that arrived
typedef struct node{
        int data;     //first node is number of seqAck than we have and others are acks recived seqs
        node *next;
     } node;

char * fileBuffer;

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

int main(char *fileName){
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
    fclose (fd);
    return 0;
  }
