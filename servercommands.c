#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "servercommands.h"

#define BUFFER_SIZE 1024
extern int errno;

/*
* Fills cmd[] with command separated into args.
*/
void parseCommand(char buffer[BUFFER_SIZE], char *cmd[]){
    //separate values into array, delimiting at " " and "\n"
    cmd[0] = strsep(&buffer, " \n");
    int i = 0;
    while (cmd[i]){
        i++;
        cmd[i] = strsep(&buffer, " \n");
    }
    //TODO: FIX TO HANDLE MESSAGES WITH SPACES
}

/*
* Returns true if file exists and false otherwise
*/
int fileExists(const char* file){
    struct stat buffer;   
    return (stat (file, &buffer) == 0);
}

/*
* Adds file to .storage directory.
* If the .storage directory doesn't exist, it is created.
* If the file already exists, it is not overwritten.
* Returns success or error code
.*/
char* addFile(char *cmd[], int argc){
    //TODO: accomodate for 3 args (no contents of file, just create blank)
    //check for proper number of args
    if (argc != 4){
        printf("invalid number of arguments\n");
        return("ERROR");
    }

    //Create .storage folder
    char path[80] = ".storage";
    int n = mkdir(path, S_IRWXU);
    if (n < 0 && errno != EEXIST){
        perror("mkdir()");
        return("ERROR");
    }

    //Construct the relative path to the file to create
    strcpy(path,"./.storage/");
    strcat(path,cmd[1]);

    //If the file already exists, return immediately
    if (fileExists(path)){
        return("FILE EXISTS");
    }

    //Otherwise, open the file and write to it
    int fd = open(path, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
    if (fd < 0){
        perror("open()");
        return("ERROR");
    } else {
        n = write(fd, cmd[3], atoi(cmd[2]));
        if (n < 0){
            perror("write()");
            return("ERROR");
        }
    }

    printf("Transferred file (%d bytes)\n", n);
    return("ACK");
}

/*
* Overwrites file in .storage directory.
* If the file doesn't exist, it is not created.
* Returns success or error code.
*/
char* updateFile(char *cmd[], int argc){
    //TODO: accomodate for 3 args (no contents, just make blank)
    //check for proper number of args
    if (argc != 4){
        printf("invalid number of arguments\n");
        return("ERROR");
    }

    //Construct the relative path to the file to create
    char path[80] = "./.storage/";
    strcat(path,cmd[1]);

    //If the file doesn't exist, return immediately
    if (!fileExists(path)){
        return("NO SUCH FILE");
    }

    int n = 0;

    //Otherwise, open the file and write to it
    int fd = open(path, O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
    if (fd < 0){
        perror("open()");
        return("ERROR");
    } else {
        int n = write(fd, cmd[3], atoi(cmd[2]));
        if (n < 0){
            perror("write()");
            return("ERROR");
        }
    }

    printf("Transferred file (%d bytes)\n", n);
    return("ACK");
}

/*
* Reads contents of file in .storage directory and stores in buffer.
* Length of read data is stored in len.
* Returns success or error code.
*/
char* readFile(char *cmd[], int argc, char* buffer, char* len){
    //check for proper number of args
    if (argc != 2){
        printf("invalid number of arguments\n");
        return("ERROR");
    }

    //Construct the relative path to the file to create
    char path[80] = "./.storage/";
    strcat(path,cmd[1]);

    //If the file doesn't exist, return immediately
    if (!fileExists(path)){
        return("NO SUCH FILE");
    }

    int n = 0;

    //Otherwise, open the file and read from it
    int fd = open(path, O_RDONLY);
    if (fd < 0){
        perror("open()");
        return("ERROR");
    } else {
        char file_contents[BUFFER_SIZE];
        n = read(fd,file_contents,BUFFER_SIZE);
        if (n < 0){
            perror("read()");
            return("ERROR");
        }
        strcpy(buffer,file_contents);
    }

    //convert len to a string
    snprintf(len, sizeof(int), "%d", n);

    printf("Transferred file (%d bytes)\n", n);
    return("ACK");
}