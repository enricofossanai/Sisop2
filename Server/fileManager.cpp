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
  char pathname[100];
  
  char usernametemp [100];
  
  char sync_dir_ [100]= "sync_dir_";
 

  printf("\nEntrou no delete\n");
  
  if (getcwd(pathname, sizeof(pathname)) != NULL) {
       printf("Current working dir: %s\n", pathname);
   } else {
       perror("getcwd() error");
       return 1;
   }

  append_dash(pathname);
  strcpy(usernametemp,username);
  append_dash(usernametemp);
  
  append_dash(sync_dir_);
  strcat(pathname,strcat(usernametemp,filename));
 
  
  

  status = remove(pathname);
  if (status == 0){
    printf("%s file deleted successfully from %s.\n", filename,username);
   return 1;
    }
  else
  {
    perror("remove");
    return -1;
  }
  
 
  
};

void append_dash(char* s) {
        int len = strlen(s);
        s[len] = '/';
        s[len+1] = '\0';
}
