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