#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <time.h>
#include <math.h>

#include "commServer.h"
#define MAX 10
using namespace std;

int checkSum(packet * packet) //verifica se o valor da soma dos dados é a mesmo( retorna 1 caso for o mesmo, -1 caso contrario)
{
    int Sum = 0,Sumchar=0,i;
    int type=packet->type,seqn=packet->seqn,lenght=packet->type,total_size=packet->total_size;

    for(i=0;i<strlen(packet->_payload);i++)
    {
        Sumchar = Sumchar + packet->_payload[i];

    }


    Sum = type + seqn + lenght + total_size + Sumchar;

    if (Sum==packet->checksum)
        return 1;
    else
    {
        return -1;
    }

}

int makeSum(packet * packet) //faz a soma dos dados do pacote
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

int usercmp(user user1, user user2){
  if ((user1.username == user2.username)&&(user1.socket == user2.socket))
      return 1;
  else
      return 0;
}


void addToONlist (user *uList, user con){
    int i = 0, added = 0;
    while (i < 10 && added == 0){
        if (uList[i].username[0] == '\0'){
          added = 1;
          //memcpy(uList+i, &con, sizeof(user));
          printf("TENTANDO:%d user:%s\n", uList[i].socket, uList[i].username);
        }
        i++;
    }

    printf("\nList of online users:\n");
    for(i = 0; i++; i<10){
        printf("n:%d user:%s\n", uList[i].socket, uList[i].username);
        fflush( stdout );
    }



    return;
}

void rmvFromONlist (user *uList, user *usr){
    int deleting = 0, i = 0;
    user aux;

    while (!usercmp(uList[i], *usr)){
        i++;
    }
    while ((i+1)<10){
        uList[i] = uList[i+1];
        i++;
    }
    uList[i] = (user){0,0,0};
    return;
}

void displayList(user *uList){
    int i;
    printf("\nList of online users:\n");
    for(i = 0; i++; i<10){
        printf("n:%d user:%s\n", uList[i].socket, uList[i].username);
        fflush( stdout );
    }
}

//NO MOMENTO SO PROPAGA PARA PRIMEIRO USUARIO
struct sockaddr_in getUserList(user *uList, user *usr){
    struct sockaddr_in cliaddrL;
    cliaddrL.sin_port = 0;
    int i = 0;

    while (i<10){
        if( (strcmp(uList[i].username, usr->username) == 0) && (uList[i].socket != usr->socket) ){
            cliaddrL = uList[i].cliSend;
            printf("SOCKET DO OUTRO : %d \n", uList[i].socket );
        }
        i++;
    }

    // If key was not present in list
    if (cliaddrL.sin_port == 0){
        printf("\n sem outra maquina de usuario conectado");
        return cliaddrL; //checar se buga, to tratando depois do retorno
    }
    return cliaddrL;
}

/*
void addToONlist (userList **list, user *con){
  if (list!=NULL){
    userList *newConnection = (userList*)malloc(sizeof(userList));
    newConnection->connection = *con;
    newConnection->next = (*list)->next;
    (*list)->next = newConnection;
    return;
  }
}


//VER SE ESTA NULL QUANDO CHAMAR PARA NAO DAR SEG FAUT
//NO MOMENTO SO PROPAGA PARA PRIMEIRO USUARIO
struct sockaddr_in getUserList(userList *list, user *usr){
    struct sockaddr_in cliaddrL;
    userList *temp = list;
    userList *found = NULL;

    cliaddrL.sin_port = 0;
    temp = temp->next;

    while (temp != NULL)
    {
        if( (strcmp(temp->connection.username, usr->username) == 0) && (temp->connection.socket != usr->socket) )
            found = temp;
        temp = temp->next;
    }

    // If key was not present in linked list
    if (found == NULL){
        printf("\n sem outra maquina de usuario conectado");
        return cliaddrL; //checar se buga, to tratando depois do retorno
    }
    printf("SOCKET DO OUTRO : %d \n", (found->connection).socket );
    // Unlink the node from linked list
    cliaddrL = found->connection.cliSend;
    return cliaddrL;
}

void rmvFromONlist (userList **list, user *usr){
    int deleted = 0;
    userList *temp = (*list), *prev;

    if (temp != NULL && temp->connection.socket == usr->socket)
     {
         (*list) = temp->next;   // Changed head
         free(temp);               // free old head
         return;
    }
    while (temp != NULL && temp->connection.socket != usr->socket)
    {
        prev = temp;
        temp = temp->next;
    }
    // If key was not present in linked list
    if (temp == NULL) return;
    // Unlink the node from linked list
    prev->next = temp->next;
    free(temp);  // Free memory
    return;
}

void displayList(userList* head){
  userList *temp;
  if(head == NULL){
    printf("List is empty.");
    fflush( stdout );
  }
  else{
    temp = head;
    temp = temp->next;
    printf("\nONLINE USERS LIST:");
    while(temp != NULL){
      printf("\nUser = %s \nIP: %d", (temp->connection).username, (temp->connection).socket); // Print data of current node
      fflush( stdout );
      temp = temp->next;                 // Move to next node
    }
    printf("\n");
  }
}
*/
int createSocket(user client, int port){
    int sockfd;
    int i,n;
    struct sockaddr_in servaddr;
    packet sendPacket;
    socklen_t len = sizeof(servaddr);


    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    //Create dir for client
    DIR* dir = opendir(client.username);
    if(dir){
        printf("Directory already exists\n");
        closedir(dir);
    }
    else{
        mkdir(client.username,0777);
        printf("Directory created\n");
        }
    memset(&servaddr, 0, sizeof(servaddr));


    printf("Criando Socket do Cliente : %s\n", client.username);

    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    sendPacket.type = ACK;
    sendPacket.cmd = 0;
    sendPacket.seqn = 0;
    sendPacket.length = 0;
    sendPacket.total_size = 0;
    strcpy(sendPacket._payload, "");
    sendPacket.checksum = checkSum(&sendPacket);

    n = sendto(sockfd, reinterpret_cast<void *> (&sendPacket), MAX_PACKET_SIZE, 0,( struct sockaddr *) &(client.cliaddr), sizeof(struct sockaddr));
    if (n < 0)
        perror("sendto");

    fflush( stdout );
    return sockfd;
}


struct sockaddr_in  getClientLSocket(user client, int sockfd){

    struct sockaddr_in cliaddrL;
    memset(&cliaddrL, 0, sizeof(cliaddrL));
	int i;
	socklen_t len = sizeof(struct sockaddr_in);

    // Filling packet for connect
    packet sentPacket, rcvdPacket;
    sentPacket.type = ACK;
    sentPacket.cmd = 0;
    sentPacket.seqn = 0;
    sentPacket.length = 0;
    strcpy(sentPacket._payload, "");
    sentPacket.total_size = 0;

      i = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &cliaddrL, &len);
      if (i  < 0)
          perror("recvfrom");
      else if(rcvdPacket.type = CNL)
          printf("Recebido pedido de Conexao de Listener Socket de usuario");

      i = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &cliaddrL, sizeof(cliaddrL));
      if (i  < 0)
          perror("sendto");

      return cliaddrL;
}



int receiveFile(char *fileName , long int fileSize,  struct sockaddr_in addr, int sockfd){
    FILE *fd = fopen( fileName , "wb" );
    unsigned char *bufferFile = (unsigned char *)malloc(fileSize + 1);
    memset(bufferFile, 0, fileSize + 1);

    if (fd == NULL){
        printf("Deu pau no arquivo\n");
        return -1;
    }

    else{
        socklen_t len = sizeof(struct sockaddr_in);

        int numSeqs = (fileSize/MAX_PAYLOAD_SIZE);
        int n;
        int curSeq = 0;
        int *allSeq = (int *)malloc((numSeqs * sizeof(int)) + 1);
        packet sentPacket, rcvdPacket;
        long bitstoReceive = 0;
        long int toWrite = fileSize;

        memset(allSeq, 0, sizeof(int) * (numSeqs + 1));

        //while still have packages to receive
        while (curSeq <= numSeqs){

            if (fileSize > MAX_PAYLOAD_SIZE)
                bitstoReceive = MAX_PAYLOAD_SIZE;
            else
                bitstoReceive = fileSize;

            n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &addr, &len);
            if(n < 0)
                perror("recvfrom");

            if((checkSum(&rcvdPacket))){		                                    // Verificação de CheckSum

                memcpy((bufferFile + (MAX_PAYLOAD_SIZE * rcvdPacket.seqn)), rcvdPacket._payload, bitstoReceive);

                sentPacket.type = ACK;
                sentPacket.seqn = curSeq;
                sentPacket.length = 0;
                sentPacket.total_size = 0;
                strcpy(sentPacket._payload, "");
                sentPacket.checksum = makeSum(&sentPacket);

                n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &addr,  sizeof(addr));
                if (n  < 0)
                    perror("sendto");

                if (allSeq[rcvdPacket.seqn] == 0){
                    fileSize = fileSize - bitstoReceive;
                    curSeq++;
                    allSeq[rcvdPacket.seqn] = 1;
                }
            }
        }

        //closes file and free the buffer
        size_t jubileu = fwrite(bufferFile, 1, toWrite, fd);
        if(jubileu != toWrite) {
            printf("Erro ao tentar escrever o arquivo inteiro.\n");
            return -1;
        }

        fclose(fd);
        free(allSeq);
        free(bufferFile);

        return 0;
    }
}


long int sizeFile (FILE *f){
	long int size;

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

int sendFile(char *fileName, struct sockaddr_in addr, int sockfd){

    FILE *fd = fopen( fileName, "rb" );
    long int fileSize = sizeFile(fd);

    int numSeqs = (fileSize/MAX_PAYLOAD_SIZE);
    int n = 0;
    int curSeq = 0;
    int curAck = 0;
    packet sentPacket, rcvdPacket;
    long int placeinBuffer = 0;
    long int bitstoSend = fileSize;
    socklen_t len = sizeof(struct sockaddr_in);
    size_t paulo;
    unsigned char *fileBuffer = (unsigned char *)malloc(fileSize);

    struct timeval timeout={2,0}; //set timeout for 2 seconds
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));


    if (fd == NULL){
        printf("Erro no arquivo");
        fclose(fd);
        return -1;
    }

    else{
        //Read file contents into buffer
        paulo = fread(fileBuffer, 1, fileSize, fd);
        if(paulo != fileSize) {
            printf("Erro ao tentar ler o arquivo inteiro.\n");
            return -1;
        }


        //while still have packages to send
    	while (curSeq <= numSeqs){

    		if (fileSize > MAX_PAYLOAD_SIZE)
        		bitstoSend = MAX_PAYLOAD_SIZE;
    		else
    			bitstoSend = fileSize;

    		//while didnt received the ack from the package
    		while (curAck == curSeq){
    	   		sentPacket.type = DATA;
    			sentPacket.seqn = curSeq;
    			sentPacket.total_size = 0;

                memcpy(sentPacket._payload, fileBuffer + placeinBuffer, bitstoSend);
                sentPacket.checksum = makeSum(&sentPacket);

    			n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, MSG_CONFIRM, (struct sockaddr *) &addr,  sizeof(addr));
    			if (n  < 0)
            		perror("sendto");

                n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, NULL, NULL);      // TEM QUE SER PAYLOAD PQ DEUS QUER
                if (rcvdPacket.seqn == curAck && n > 0){
    		    	    curAck++;
                }
    		}

    		placeinBuffer = placeinBuffer + bitstoSend; //move the place of the flag in the buffer for next packet
    		fileSize = fileSize - bitstoSend;
    		curSeq++;
    	}
  }

  struct timeval notimeout={0,0}; //set timeout for 2 seconds
  setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&notimeout,sizeof(struct timeval));

  free(fileBuffer);
  fclose(fd);

  return 0;
}

//sends message to create, delete or modify file
void send_cmd(char *fileName, struct sockaddr_in addr, int sockfd, int command, char *dir){
  //filling packet info
    socklen_t len = sizeof(struct sockaddr_in);
    packet sentPacket, rcvdPacket;
    memset(&sentPacket, 0 , sizeof(struct packet));
    sentPacket.type = CMD;
    sentPacket.cmd = command;
    strcpy(sentPacket._payload,fileName);


    int n;
    //struct timeval timeout={2,0}; //set timeout for 2 seconds
    //setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

    //sending packet
    int ack = 0;

    if (command == CREATE || command == MODIFY){
        sleep(3);
        FILE *fd = fopen( dir, "rb" );
        sentPacket.length = sizeFile(fd);
        fclose(fd);
    }

    sentPacket.checksum = makeSum(&sentPacket);

    while (ack == 0){
            //printf("Enviando mensagem\n") ;
        n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &addr,  sizeof(addr));
        if (n  < 0)
            perror("sendto");
            //printf("Esperando Ack\n") ;
        n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &addr, &len);
        if (n  < 0)
            perror("recvfrom");
        if (checkSum(&rcvdPacket)){
            ack = 1;
            printf("\nClient received command\n");
        }
    }
    //struct timeval timeout={0,0}; //set timeout to return to block
    //setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

    fflush(stdout);
    return;
}

cmdAndFile rcv_cmd(struct sockaddr_in addr, int sockfd){
  //filling packet info
    socklen_t len = sizeof(struct sockaddr_in);
    packet sentPacket, rcvdPacket;
    int n;
    cmdAndFile returnFile;
    returnFile.command = -1;
    //printf("\nEsperando Comando...");
    //fflush(stdout);
    n = recvfrom(sockfd, reinterpret_cast<void *> (&rcvdPacket), MAX_PACKET_SIZE, 0, NULL, NULL);
    if (n  < 0)
        perror("recvfrom");
    if (checkSum(&rcvdPacket)){
        //printf("\nserver received command %d\n", rcvdPacket.cmd);
        sentPacket.type = ACK;
        sentPacket.cmd = rcvdPacket.cmd;
        sentPacket.checksum = makeSum(&sentPacket);
        n = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *) &addr,  sizeof(addr));
        fflush(stdout);
        if (n  < 0)
            perror("sendto");
        returnFile.command = rcvdPacket.cmd;
        strcpy(returnFile.fileName, rcvdPacket._payload);
        returnFile.fileSize = rcvdPacket.length;

        return returnFile;
      } else{
            printf("\nERRO DE CHECKSUM NO  COMANDO");
            fflush(stdout);
          }
    //struct timeval timeout={0,0}; //set timeout to return to block
    //setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

    return returnFile;
}

void make_cmd (cmdAndFile lastCommand, user *client, char *dirClient, user *uList){
    int n;
    packet sendPacket;
    char buffer[MAX_PAYLOAD_SIZE];
    socklen_t len = sizeof(struct sockaddr_in);
    char file[100] = {};
    struct sockaddr_in destiny;

    bzero(file, 100);
    strcpy(file, dirClient);

    if(lastCommand.command == CREATE) {
        printf("\nRECEIVED CREATE FILE COMMAND WITH SIZE: %ld", lastCommand.fileSize);
        strcat(file, lastCommand.fileName);
        n =  receiveFile( file , lastCommand.fileSize, client->cliaddr,client->socket );

        destiny = getUserList(uList, client);
        if (destiny.sin_port != 0){
            send_cmd(lastCommand.fileName, destiny, client->socket, CREATE, file);
            sendFile(file , destiny, client->socket);
        }
    }
    else if(lastCommand.command == DELETE) {
        printf("\nRECEIVED DELETE FILE COMMAND");
        n = delete_file(lastCommand.fileName,client->username);

        destiny = getUserList(uList, client);
        if (destiny.sin_port != 0)
        send_cmd(lastCommand.fileName, destiny, client->socket, DELETE, NULL);

      }
    else if (lastCommand.command == MODIFY){
        printf("\nRECEIVED MODIFY FILE COMMAND");
        n = delete_file(lastCommand.fileName, client->username);
        strcat(file, lastCommand.fileName);
        n =  receiveFile( file , lastCommand.fileSize, client->cliaddr,client->socket );

        destiny = getUserList(uList, client);
        if (destiny.sin_port != 0){
            send_cmd(lastCommand.fileName, destiny, client->socket, MODIFY, file);
            sendFile(file , destiny, client->socket);
        }
    }
      else if (lastCommand.command ==LIST_SERVER){
        printf("\nRECEIVED LIST_SERVER COMMAND");
            if (list_server(client->username, buffer)){
                fflush(stdout);
                strcpy(sendPacket._payload,buffer);
                sendPacket.type = DATA;
                sendPacket.checksum = makeSum(&sendPacket);
                printf("\nEnviando lista de arquivos\n");
                n = sendto(client->socket, reinterpret_cast<void *> (&sendPacket), MAX_PACKET_SIZE, 0, ( struct sockaddr *)  &(client->cliaddr), sizeof(client->cliaddr));
                if (n  < 0)
                    perror("sendto");
                fflush(stdout);
            }
      }
      else if (lastCommand.command == EXIT){
        printf("\nRECEIVED LIST_SERVER EXIT");
        rmvFromONlist (uList, client);
        displayList(uList);
      }
      else if (lastCommand.command == DOWNLOAD){
            printf("\nRECEIVED DOWNLOAD COMMAND");
            strcat(file, lastCommand.fileName);
            printf("FILE : %s\n", file);

            FILE *fd = fopen( file, "rb" );
            sendPacket.length = sizeFile(fd);

            sendPacket.checksum = makeSum(&sendPacket);

            n = sendto(client->socket, reinterpret_cast<void *> (&sendPacket), MAX_PACKET_SIZE, 0, ( struct sockaddr *)  &(client->cliaddr), sizeof(client->cliaddr));
            if (n  < 0)
                perror("sendto");
            n =  sendFile( file , client->cliaddr,client->socket );
      }
}

void connectBackup (int sockfd , struct hostent *server, int servType){
	struct sockaddr_in servaddr;
	int i;
	socklen_t len = sizeof(struct sockaddr_in);

	memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr = *((struct in_addr *)server->h_addr);

    // Filling packet for connect
    packet sentPacket, recPacket;
    sentPacket.type = servType;
    sentPacket.cmd = 0;
    sentPacket.seqn = 0;
    sentPacket.length = 0;
    sentPacket.total_size = 0;
    strcpy(sentPacket._payload, "");
    sentPacket.checksum = makeSum(&sentPacket);

    i = sendto(sockfd, reinterpret_cast<void *> (&sentPacket), MAX_PACKET_SIZE, 0, (const struct sockaddr *) &servaddr,  sizeof(servaddr));
	if (i  < 0)
        perror("sendto");

    i = recvfrom(sockfd, reinterpret_cast<void *> (&recPacket), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &servaddr, &len);
    if (i  < 0)
        perror("recvfrom");

}

int makeElection ( struct sockaddr_in electlist[],struct sockaddr_in servaddr,int ID,int socksd){

    int i,n,node;
    packet packet;
    struct sockaddr_in send;
    int size = sizeof(struct sockaddr_in);

    for(i=0;i<MAX;i++){

        if(electlist[i].sin_port== servaddr.sin_port){

            node = i;
            packet.type = DATA;
            strcpy(packet._payload, "Election," + ID);
            if(node == MAX-1)
                node = -1;
            send = electlist[node+1];
            n = sendto(socksd, reinterpret_cast<void *> (&packet), MAX_PACKET_SIZE, 0, (struct sockaddr *)  &(send), size);
            if(n < 0)
                perror("sendto");

        }
    }

}