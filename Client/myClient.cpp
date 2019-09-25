//Headers
#include "aplication.h"
#include "commClient.h"

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
#include <pthread.h>
#include <bits/stdc++.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>


//global variables
struct sockaddr_in servaddr;
int sockfd;
char * fileBuffer;

// Driver code
int main(int argc, char *argv[]) {


//////USANDO AREA PARA TESTAR PODE APAGAR DEPOIS
//fileBuffer = "me diz que funfou";
///////////////////////////////////////////////


    int i,flag=FALSE;
	char username[20],command[20],option[20];
    char buffer[MAX_PACKET_SIZE];

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

	firstConnect(sockfd,server);

    //cria thread que envia
    pthread_t threadSender;
    pthread_create(&threadSender, NULL, sender, NULL);

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

void *sender(void *arg) {
    printf("Thread is listening!\n");

    char buffer[MAX_PACKET_SIZE];
    socklen_t len = sizeof(servaddr);
	int n;

    n = recvfrom(sockfd, (char *)buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *) &servaddr, &len);
    if (n  < 0)
        printf("\nERROR on rcvfrom\n");

    printf("Server : %s\n", buffer);
    fflush( stdout );
}
