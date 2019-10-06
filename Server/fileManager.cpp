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

char * list_server(char *dirName){

    DIR *dir;
    struct dirent *dent;
    dir = opendir((const char *) dirName);

    if(dir!=NULL)
    {
        while((dent=readdir(dir))!=NULL){
            struct stat info;
            stat(dent->d_name, &info);

            printf("Arquivo:%s\n-Modification Time: %.12s\n-Access Time:%.12s\n-Creation Time:%.12s \n",dent->d_name,4+ctime(&info.st_mtime),4+ctime(&info.st_atime),4+ctime(&info.st_ctime));
        }


        return 1;
    }
    else{
        printf("Erro na abertura do diretório\n");
        return -1;
        }
    fflush(stdout);
    closedir(dir);


}