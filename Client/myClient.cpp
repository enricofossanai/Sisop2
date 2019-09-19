//Headers
#include "aplication.h"
#include "communication.h"
#include "sync.h"

// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
  
#define PORT     8000 
#define MAXLINE 102400 
  
// Driver code 
int main() { 
    int sockfd; 
    char buffer[MAXLINE]; 
    char *hello = "Hello from client"; 
    struct sockaddr_in     servaddr; 
  
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    memset(&servaddr, 0, sizeof(servaddr)); 
   
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
      
    int n; 
    socklen_t len = sizeof(servaddr);

    
    packet sentPacket;
    sentPacket.type = 0;
    sentPacket.seqn = 1;
    sentPacket.length = 42;
    sentPacket.total_size = 2;
    sentPacket.checksum = 3;
    sentPacket._payload = "oi";

    
    char * message = "conectando";
   // marshallPacket(&sentPacket, message);
    printf("Depois do Marshall");
    fflush( stdout );

    sendto(sockfd, (const void *) message, sizeof(message), MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr)); 
    printf("Packet sent.\n"); 
    fflush( stdout );
          
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len); 

    printf("Server : %s\n", buffer); 
    fflush( stdout );
  
    close(sockfd); 
    return 0; 
} 
