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

#include "fileManager.h"

int list_server(char *dirName, char * serverFolderSatus ){

    DIR *dir;
    struct dirent *dent;
    dir = opendir((const char *) dirName);
    
     

    if(dir!=NULL)
    {
        while((dent=readdir(dir))!=NULL){
            struct stat info;
            stat(dent->d_name, &info);
            strcat(serverFolderSatus,"Arquivo:");
            strcat(serverFolderSatus,dent->d_name);
            strcat(serverFolderSatus,"\n-Modification Time:");
            strcat(serverFolderSatus,4+ctime(&info.st_mtime));
            strcat(serverFolderSatus,"\n-Access Time:");
            strcat(serverFolderSatus,4+ctime(&info.st_atime));
            strcat(serverFolderSatus,"\n-Creation Time:");
            strcat(serverFolderSatus,4+ctime(&info.st_ctime));

            
        }

        //printf("%s",serverFolderSatus);
        return 1;

    }
    else{
        printf("Erro na abertura do diretório\n");
        return -1;
        }
    fflush(stdout);
    closedir(dir);


}

int delete_file(char * filename,char * username){

  int status;
  char  pathname[100];
  char  buffer[100];
  char usernametemp [100];
 

  printf("\nEntrou no delete\n");
  
  if (getcwd(buffer, sizeof(buffer)) != NULL) {
       printf("Current working dir: %s\n", buffer);
   } else {
       perror("getcwd() error");
       return 1;
   }

  append_dash(buffer);
  strcpy(usernametemp,username);
  append_dash(usernametemp);
  strcat(buffer,strcat(usernametemp,filename));
  strcpy(pathname,buffer);
  fprintf(stderr,"pathname1: %s\n",pathname);
  //fprintf(stderr,"pathname1: /home/rodolfo/Sisop2/Sisop2/bin/rodolfin/a.png\n");
  fflush(stdout);
  //status = strcmp(pathname,"/home/rodolfo/Sisop2/Sisop2/bin/rodolfin/a.png");
  //printf("%d",status);

  status = remove(pathname);
  if (status == 0){
    printf("%s file deleted successfully.\n", filename);
    return 1;}
  else
  {
    fprintf(stderr,"Unable to delete the file\n");
    return -1;
  }
  
 
  
};

void append_dash(char* s) {
        int len = strlen(s);
        s[len] = '/';
        s[len+1] = '\0';
}