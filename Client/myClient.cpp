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
int sockfd;
struct hostent *server;
char username[20];
//mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
//mutex initialization flag
  if (pthread_mutex_init(&mutex, NULL) != 0)
  {
      printf("\n mutex init failed\n");
      return 1;
  }

    int i,flag=FALSE,status;
	char dirName[100],command[20],option[20], sync_dir[40],filename[40];
    char buffer[MAX_PACKET_SIZE];




    if (argc < 3) {
		fprintf(stderr, "usage %s hostname username\n", argv[0]);
		exit(0);

	}

    strcpy (sync_dir, "sync_dir_");
    strcpy (username,argv[2]);
    strcpy(dirName,strcat(sync_dir, username));

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

	servaddr = firstConnect(sockfd,server,username, 0);                       // Conecta com o famigerado Servidor

    //cria thread que envia
    pthread_t threadSender;
    pthread_create(&threadSender, NULL, clientComm, NULL );                  // Inicia a thread
    pthread_detach(threadSender);

    pthread_t threadN;
    pthread_create(&threadN, NULL, clientNotify, (void *) sync_dir);

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

        } else if (strcmp(command, "upload\n") == 0) { // upload from path
            printf("\nUPLOAD command chosen\n");
            printf("\nEnter the file name: ");
            fflush(stdout);
            bzero(filename, 40);

            scanf("%s", filename);

            pthread_mutex_lock(&mutex);
            send_cmd(filename, servaddr, sockfd, CREATE, filename);
            sendFile(filename , servaddr, sockfd);
            pthread_mutex_unlock(&mutex);

        } else if (strcmp(command, "download\n") == 0) { // download to exec folder
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

        } else if (strcmp(command, "delete\n") == 0) { // delete from syncd dir
            printf("\nDELETE command chosen\n");
            printf("\nEnter the file name: ");
            fflush(stdout);
            bzero(filename, 40);
            scanf("%s", filename);
            bzero(dirName, 100);
            getcwd(dirName, sizeof(dirName));
            strcat(dirName, "/");
            strcat(dirName,sync_dir);
            strcat(dirName,"/");
            strcat(dirName,filename);
            printf("%s",dirName);
            status = remove(dirName);
            if (status == 0){
                printf("%s file deleted successfully from sync_dir_%s.\n", filename,username);
                }
            pthread_mutex_lock(&mutex);
            send_cmd(filename, servaddr, sockfd, DELETE, NULL);
            pthread_mutex_unlock(&mutex);
        } else if (strcmp(command, "list_server\n") == 0) { // list user's saved files on dir
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

        } else if (strcmp(command, "list_client\n") == 0) { // list saved files on dir
            printf("\nLIST_CLIENT command chosen\n");
            i = list_client(dirName);
            if (i  > 0)
                printf("Leu o diretório\n");
            else
                printf("Erro no list_client\n");
            fflush(stdout);

        } else if (strcmp(command, "get_sync_dir\n") == 0) { // creates sync_dir_<username> and syncs
            printf("\nGET_SYNC_DIR command chosen\n");
        }
    }

    close(sockfd);

    return 0;
}

void *clientComm(void *arg) {
    packet recPacket;
    int cliSock;
    char buffer[MAX_PACKET_SIZE];
    socklen_t len = sizeof(servaddr);
	int n;
/*
    while(0){

    printf("Esperando Mensagem\n") ;
    servaddr = firstConnect(cliSock,server,username, 1);

    lastCommand = rcv_cmd(servaddr, cliSock);

    if (lastCommand.command >= 0){ // if recieved command wasnt corrupted
        if(lastCommand.command == CREATE) {
            printf("\nRECEIVED CREATE FILE COMMAND WITH SIZE: %ld", lastCommand.fileSize);
            strcat(file, lastCommand.fileName);
            n =  receiveFile( file , lastCommand.fileSize, servaddr , cliSock );
        }
        else if(lastCommand.command == DELETE) {
            printf("\nRECIEVED DELETE FILE COMMAND");
            n = delete_file(lastCommand.fileName, username);
        }
        else if (lastCommand.command == MODIFY){
            printf("\nRECIEVED MODIFY FILE COMMAND");
            // delete (recPacket._payload)
            // Recebe o nome do arquivo, apaga da base do Servidor
        }

    fflush( stdout );
    }
*/
}

void *clientNotify(void *arg){
    int fd, wd ;
    char buf[1024 * (sizeof(struct inotify_event))];
    int i, t, l ;
    fd_set rfds ; /* para select */
    struct inotify_event *evento ;
    char dirName [100];



    if((fd = inotify_init())<0) {
        perror("inotify_init") ;
    }

    wd = inotify_add_watch(fd, (char *) arg, IN_MOVED_FROM | IN_MODIFY | IN_DELETE | IN_CREATE | IN_MOVED_TO) ;           // ADICIONAR AS FLAGS CERTAS
    if(wd < 0) {
        perror("inotify_add_watch") ;
    }

    while(1) {
        bzero(dirName, 100);
        strcpy(dirName, "./");
        strcat(dirName, (char *) arg);
        strcat(dirName, "/");

        FD_ZERO(&rfds) ;
        FD_SET(fd, &rfds) ;

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

        /* Percorre cada evento lido. */
        i=0 ;
        while(i<l) {
            /* Obtém dados na forma da struct. */
            evento = (struct inotify_event *)&buf[i] ;

            /* Se o campo len não é nulo, então temos
             * um nome no campo name. */
            if(evento->len) {
                printf("[+] Arquivo `%s': ", evento->name) ;
            } else {
                printf("[+] Arquivo desconhecido: ") ;                               // Nome do Arquivo modificado
            }

            strcat(dirName, evento->name);

            /* Obtém o evento. */
            if(evento->mask & IN_MODIFY)     {                                        // SOFRE O PROBLEMA DO GEDIT
                printf("\nModificado.\n") ;
            //    send_cmd(evento->name , servaddr, sockfd, MODIFY, dirName);
            } else if(evento->mask & IN_DELETE || evento->mask & IN_MOVED_FROM) {    // DELETE SOFRE O PROBLEMA DO UBUNTU
                printf("\nDeletado.\n") ;
                pthread_mutex_lock(&mutex);
                send_cmd(evento->name , servaddr, sockfd, DELETE, dirName);
                pthread_mutex_unlock(&mutex);
            } else if(evento->mask & IN_CREATE || evento->mask & IN_MOVED_TO){
                printf("\nCriado.\n") ;
                pthread_mutex_lock(&mutex);
                send_cmd(evento->name, servaddr, sockfd, CREATE, dirName);
                sendFile(dirName , servaddr, sockfd);
                pthread_mutex_unlock(&mutex);
            }

            /* Avança para o próximo evento. */
            i += (sizeof(struct inotify_event)) + evento->len ;
        }
    }
}
