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
#include <netdb.h>
#include <bits/stdc++.h> 
#include <iostream> 
#include <sys/stat.h> 
#include <sys/types.h> 

<<<<<<< HEAD
#define PORT     8000
#define MAXLINE 102400
#define MAX_PACKET_SIZE 64000
#define TRUE 1
#define FALSE 0

// Driver code
int main(int argc, char *argv[]) {
    int sockfd, i,flag=FALSE;
    char buffer[MAXLINE];
    char username[20],command[20],option[20];
=======
#define PORT  8000

// Driver code
int main(int argc, char *argv[]) {
    int sockfd, i;
    char buffer[MAX_PACKET_SIZE];
    char *hello = "Hello from client";
>>>>>>> d36a1feb5d60732dca4cc9b1034048fa0e1a2848
    struct sockaddr_in servaddr;
    struct hostent *server;

    if (argc < 3) {
		fprintf(stderr, "usage %s hostname username\n", argv[0]);
		exit(0);

	}

    strcpy (username,argv[2]);

    if (!(mkdir(username,0777))) 
        printf("Directory created\n"); 
    else { 
        printf("Unable to create directory\n"); 
        exit(0); 
    } 

    server = gethostbyname(argv[1]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

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
    sentPacket.type = 5;
    sentPacket.seqn = 100;
    sentPacket.length = 42;
    sentPacket.total_size = 7;
    strcpy(sentPacket._payload, "PRA QUE MARSHLING NESSA CACETA ENRICAO");
    sentPacket.checksum = checkSum(&sentPacket);

    fflush(stdout);
    memcpy(buffer, &sentPacket, sizeof(buffer));


    char * message = "conectando";


    sendto(sockfd, (const void *) buffer, MAX_PACKET_SIZE, MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr));  // Precisa arrumar o tamanho do que ta enviando
    printf("Packet sent.\n");                                                                                                     // 70 é só um numero cabalistico
    fflush( stdout );

<<<<<<< HEAD
=======
    n = recvfrom(sockfd, (char *)buffer, MAX_PACKET_SIZE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
>>>>>>> d36a1feb5d60732dca4cc9b1034048fa0e1a2848

    while (flag == FALSE) {

        printf("\nEnter the Command: ");
        bzero(command, 20);
        fgets(command, 20, stdin);


        // Switch for options
        if(strcmp(command,"exit\n") == 0) {
            flag = TRUE;
        } else if (strcmp(command, "upload") == 0) { // upload from path
            
        } else if (strcmp(command, "download") == 0) { // download to exec folder
            
        } else if (strcmp(command, "delete") == 0) { // delete from syncd dir
            
        } else if (strcmp(command, "list_server") == 0) { // list user's saved files on dir
            
        } else if (strcmp(command, "list_client") == 0) { // list saved files on dir
            
        } else if (strcmp(command, "get_sync_dir") == 0) { // creates sync_dir_<username> and syncs
            
        } else if (strcmp(command, "printar") == 0) { // creates sync_dir_<username> and syncs
        }

    }

    close(sockfd);
    return 0;
}
