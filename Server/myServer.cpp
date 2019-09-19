//Headers
#include "communication.h"
#include "fileManager.h"
#include "sync.h"

// Server side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <pthread.h>
#include <vector>
  

using namespace std;

#define PORT     8000 
#define MAXLINE 102400
#define MAXNUMCON 100
  
//header da thread foda-se
void *do_it_1(void *arg);


// Driver code 
int main() { 
    int sockfd; 
    char buffer[MAXLINE]; 
    char *hello = "Hello from server"; 
    struct sockaddr_in servaddr, cliaddr; 
      
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    vector<pthread_t> threads(MAXNUMCON);
    int threadNum = 0;
    int rc;

    while(1){
            packet packetBuffer;
            int n;
            socklen_t len = sizeof(servaddr); 
            n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);


            char* userName = "jucaBatista";

            rc = pthread_create(&threads[threadNum], NULL, connect, (void*)userName);
    }

    /*
    unmarshallPacket(&packetBuffer, buffer);

    printf("Client : %hu\n", packetBuffer.type); 
    printf("Client : %u\n", packetBuffer.total_size);
    printf("Client : %s\n", packetBuffer._payload);

    sendto(sockfd, (const char *)hello, strlen(hello),  
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
            len); 
    printf("Hello message sent.\n");  
    */

    return 0; 
} 


//thread executada toda vez que abre uma 
void *connect(void *arg) {
    printf("Thread Created Successfully!\n");
}