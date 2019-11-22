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
#include <dirent.h>


//global variables
struct sockaddr_in servaddr;
int sockfd, sockfdL;
struct hostent *server;
char dirName[100], username[20];
int notify_block = 0;

//mutex
pthread_mutex_t mutex;

int main(int argc, char *argv[]) {

    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }


    int i,flag=FALSE,status;
	char command[20],option[20], sync_dir[40],filename[40], userfile[40];
    char buffer[MAX_PACKET_SIZE];

    if (argc < 3) {
		fprintf(stderr, "usage %s hostname username\n", argv[0]);
		exit(0);
	}

    strcpy (sync_dir, "sync_dir_");
    strcpy (username,argv[2]);
    strcpy(dirName,strcat(sync_dir, username));

	strcpy(userfile, dirName);

    //Cria diretório sync_dir_username caso ele ainda nao exista
    DIR* dir = opendir(dirName);
    if(dir){
        printf("Directory already exits\n");
        closedir(dir);
    }
    else{
        mkdir(dirName,0777);
        printf("Directory created\n");
        }
                                                                            // Cria o Diretório

    server = gethostbyname(argv[1]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");                            // Coisa do server
        exit(0);
    }

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {                  // Coisa do Socket
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    if ( (sockfdL = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("listener socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr = firstConnect(sockfd,server,username);                       // Conecta com o famigerado Servidor
    sockfdL = connectListener(sockfdL,servaddr,username);                 //conecta o socket da thread que escuta tbm com o server


    //cria thread que envia
    pthread_t threadSender;
    pthread_create(&threadSender, NULL, clientComm, (void *) sockfdL );                  // Inicia a thread

    pthread_t threadN;
    pthread_create(&threadN, NULL, clientNotify, (void *) sync_dir);                    // Thread do Notify

    while (flag == FALSE) {

        printf("\nEnter the Command: ");
        fflush(stdout);                                                     //////////////////////////////////////////////
        bzero(command, 20);                                                 // Será que o menu não é dentro da thread ????
        fgets(command, 20, stdin);                                          //////////////////////////////////////////////


        // Switch for options
        if(strcmp(command,"exit\n") == 0) {
            flag = TRUE;

            pthread_mutex_lock(&mutex);
            send_cmd("", servaddr, sockfd, EXIT, NULL);
            pthread_mutex_unlock(&mutex);

        }
        else if (strcmp(command, "upload\n") == 0) { // upload from path
            printf("\nUPLOAD command chosen\n");
            printf("\nEnter the file name: ");
            fflush(stdout);
            bzero(filename, 40);

            scanf("%s", filename);
            strcat(dirName, "/");

            pthread_mutex_lock(&mutex);

            copyFile(filename, strcat(dirName, filename));

            pthread_mutex_unlock(&mutex);

        }
        else if (strcmp(command, "download\n") == 0) { // download to exec folder
            printf("\nDOWNLOAD command chosen\n");
            printf("\nEnter the file name: ");
            fflush(stdout);
            bzero(filename, 40);
            scanf("%s", filename);

            pthread_mutex_lock(&mutex);

            send_cmd(filename, servaddr, sockfd, DOWNLOAD, NULL);

            packet recPacket;
            socklen_t len = sizeof(struct sockaddr_in);

            i = recvfrom(sockfd, reinterpret_cast<void *> (&recPacket), MAX_PACKET_SIZE, 0, NULL,  NULL);
            if (i < 0)
                perror("recvfrom");
            if(!checkSum(&recPacket))
                perror("erro checksum");

            i =  receiveFile( filename ,recPacket.length ,servaddr ,sockfd );

            pthread_mutex_unlock(&mutex);

        }
        else if (strcmp(command, "delete\n") == 0) { // delete from syncd dir
            printf("\nDELETE command chosen\n");
            printf("\nEnter the file name: ");
            fflush(stdout);
            bzero(filename, 40);
            scanf("%s", filename);

            pthread_mutex_lock(&mutex);
            send_cmd(filename, servaddr, sockfd, DELETE, NULL);
            pthread_mutex_unlock(&mutex);
        }
        else if (strcmp(command, "list_server\n") == 0) { // list user's saved files on dir
            printf("\nLIST_SERVER command chosen\n");
            packet recPacket;
            socklen_t len = sizeof(struct sockaddr_in);
            pthread_mutex_lock(&mutex);
            send_cmd("",servaddr,sockfd,LIST_SERVER, NULL);
            i = recvfrom(sockfd, reinterpret_cast<void *> (&recPacket), MAX_PACKET_SIZE, 0, ( struct sockaddr *)  &servaddr,  &len);
            pthread_mutex_unlock(&mutex);
            if (i < 0)
                perror("recvfrom");
            if(!checkSum(&recPacket))
                perror("erro checksum");

            printf("%s",recPacket._payload);
            fflush(stdout);

        }
        else if (strcmp(command, "list_client\n") == 0) { // list saved files on dir
            printf("\nLIST_CLIENT command chosen\n");
            i = list_client(userfile);
            if (i  > 0)
                printf("Leu o diretório\n");
            else
                printf("Erro no list_client\n");
            fflush(stdout);

        }
        else if (strcmp(command, "get_sync_dir\n") == 0) { // creates sync_dir_<username> and syncs
            printf("\nGET_SYNC_DIR command chosen\n");
        }
        else if (strcmp(command, "teste\n") == 0){
            packet sendPacket;
            i = sendto(sockfd, reinterpret_cast<void *> (&sendPacket) , MAX_PACKET_SIZE, 0, (const struct sockaddr *) &servaddr,  sizeof(servaddr));
            if (i < 0)
                perror("sendto");
        }
    }

    pthread_mutex_destroy(&mutex);
    close(sockfd);

    return 0;
}

void *clientComm(void *arg) {
    packet recPacket;
    int cliSock = arg;
    char buffer[MAX_PACKET_SIZE];
    socklen_t len = sizeof(servaddr);
	int n;
    cmdAndFile lastCommand;
    char file [100];

    while(1){
        bzero(file , 100);
        strcpy(file, dirName);
        strcat(file, "/");
        // printf("Esperando Mensagem\n") ;

        lastCommand = rcv_cmd(servaddr, cliSock);

        pthread_mutex_lock(&mutex);

        notify_block = 1;

        if (lastCommand.command >= 0){                      // if received command wasnt corrupted
            if(lastCommand.command == CREATE) {
                //printf("\nRECEIVED CREATE FILE COMMAND WITH SIZE: %ld", lastCommand.fileSize);
                fflush(stdout);
                strcat(file, lastCommand.fileName);

                //printf("FILE : %s\n", file);
                n =  receiveFile( file , lastCommand.fileSize, servaddr , cliSock );

            }
            else if(lastCommand.command == DELETE) {
                //printf("\nRECEIVED DELETE FILE COMMAND");
                strcat(file, lastCommand.fileName);
                n = remove(file);
                if (n == 0);
                  //printf("%s file deleted successfully from %s.\n", file,username);
                else
                  perror("remove");

            }
            else if (lastCommand.command == MODIFY){
                //printf("\nRECEIVED MODIFY FILE COMMAND");
                strcat(file, lastCommand.fileName);
                n = remove(file);
                if (n == 0);
                  //printf("%s file deleted successfully from %s.\n", file,username);
                else
                  perror("remove");

                n =  receiveFile( file , lastCommand.fileSize, servaddr , cliSock );
            }
            else if (lastCommand.command == SERVER){

                n = recvfrom(sockfd, reinterpret_cast<void *> (&recPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &servaddr, &len);
                if (n < 0)
                    perror("recvfrom");

                notify_block = 0;
            }
        }

        pthread_mutex_unlock(&mutex);
        fflush( stdout );
    }

}

void *clientNotify(void *arg){
    int fd, wd ;
    char buf[1024 * (sizeof(struct inotify_event))];
    int i, t, l ;
    fd_set rfds ; /* para select */
    struct inotify_event *evento ;
    char dirName [100];
    int justCreated = 0;

    if((fd = inotify_init())<0) {
        perror("inotify_init") ;
    }

    wd = inotify_add_watch(fd, (char *) arg, IN_MOVED_FROM | IN_MODIFY | IN_DELETE | IN_CREATE | IN_MOVED_TO) ;           // ADICIONAR AS FLAGS CERTAS
    if(wd < 0) {
        perror("inotify_add_watch") ;
    }

    while(1) {

        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);


        // Verifica se há dados no descritor
        t = select(FD_SETSIZE, &rfds, NULL, NULL, NULL) ;
        if(t<0) {
            perror("select") ;
        }
        /* Sem dados. */
        if(t == 0) continue ;
        /* Aqui temos dados. */

        /* Lê o máximo de eventos. */
        l = read(fd, buf, 1024 * (sizeof(struct inotify_event))) ;
        //printf("MUTEX : %d\n", notify_block );




        /* Percorre cada evento lido. */
        i=0 ;

        pthread_mutex_lock(&mutex);

        while(i<l) {
            bzero(dirName, 100);
            strcpy(dirName, "./");
            strcat(dirName, (char *) arg);
            strcat(dirName, "/");
            /* Obtém dados na forma da struct. */
            evento = (struct inotify_event *)&buf[i] ;

            /* Se o campo len não é nulo, então temos
             * um nome no campo name. */
            if(evento->len) {
               // printf("[+] Arquivo `%s': ", evento->name) ;
            } else {
               // printf("[+] Arquivo desconhecido: ") ;                               // Nome do Arquivo modificado
            }

            strcat(dirName, evento->name);

            /* Obtém o evento. */
            if(evento->mask & IN_MODIFY)     {                                        // SOFRE O PROBLEMA DO GEDIT
                if(justCreated == 0 && notify_block == 0){
                    //printf("\nModificado.\n") ;
                    send_cmd(evento->name , servaddr, sockfd, MODIFY, dirName);
                    sendFile(dirName , servaddr, sockfd);
                    justCreated = 1;
                }
                else{
                    justCreated = 0;
                }
            }
            else if(evento->mask & IN_DELETE || evento->mask & IN_MOVED_FROM ) {    // DELETE SOFRE O PROBLEMA DO UBUNTU
                        //printf("\nDeletado.\n") ;
                        if(notify_block == 0)
                            send_cmd(evento->name , servaddr, sockfd, DELETE, dirName);
            }
            else if(evento->mask & IN_CREATE || evento->mask & IN_MOVED_TO){
                    justCreated = 1;
                    if(notify_block == 0){
                        //printf("\nCriado.\n") ;
                        send_cmd(evento->name, servaddr, sockfd, CREATE, dirName);
                        sendFile(dirName , servaddr, sockfd);
                    }
            }

            /* Avança para o próximo evento. */
            i += (sizeof(struct inotify_event)) + evento->len ;

            if (notify_block == 1)
                notify_block = 0;

            pthread_mutex_unlock(&mutex);
        }


    }
}
