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
* Returns true if file exists and false otherwise
*/
int fileExists(const char* file){
    struct stat buffer;   
    return (stat (file, &buffer) == 0);
}

/*
* Parses the filename out of cmd and puts it in filename
*/
void parseFilename(char cmd[100], char* filename){
    int i = 0;  //ctr for location in cmd[]
    int filename_flag = 0;  //flag for if current char is part of filename
    char temp[2];   //used to hold each char of cmd
    while(cmd[i]){
        temp[0] = cmd[i];
        temp[1] = '\0';
        //at the first space, set the flag. at the second, return.
        if (strcmp(temp," ") == 0){
            if (filename_flag) return;
            filename_flag = 1;
        }
        if (filename_flag && strcmp(temp," ") != 0){
            strcat(filename, temp);
        }
        i++;
    }
}

/*
* Parses size from cmd, returns as int
*/
int parseSize(char* cmd){
    char* size = malloc(100*sizeof(char));
    int i = 0;
    int size_flag = 0;
    char temp[2];
    while(cmd[i]){
        temp[0] = cmd[i];
        temp[1] = '\0';
        if (size_flag == 2){
            strcat(size, temp);
        } else if (strcmp(temp," ") == 0){
            size_flag++; 
        }
        i++;
    }

    int bytes = atoi(size);
    return bytes;
}

/*
* Helper function to check for prefix matching in strings.
* Returns 1 if the first n characters in string match the
* characters in searchstring (which is of length n).
* Otherwise returns 0.
*/
int prefixMatch(char* string, char* searchstring){
  int i = 0;
  while (i < strlen(searchstring)){
    if (string[i] != searchstring[i]){
      return 0;
    }
    i++;
  }
  return 1;
}

/*
* Adds file to .storage directory.
* If the .storage directory doesn't exist, it is created.
* If the file already exists, it is not overwritten.
* Returns success or error code
* Args:
*   size: number of bytes to write
*   filename: name of file to write to
*   data: string array containing data to write
*   thread_id: id# of thread that called the function
.*/
char* addFile(int size, char* filename, char** data, int thread_id){
    //Create .storage folder
    char path[80] = ".storage";
    int n = mkdir(path, S_IRWXU);
    if (n < 0 && errno != EEXIST){
        perror("mkdir()");
        return("ERROR");
    }

    //Construct the relative path to the file to create
    strcpy(path,"./.storage/");
    strcat(path,filename);

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
        int bytes_to_write;
        if (size > 1024) bytes_to_write = 1024;
        else bytes_to_write = size;

        n = write(fd, data[0], bytes_to_write);

        if (n < 0){
            perror("write()");
            return("ERROR");
        }
    }
    close(fd);
    
    if(size < BUFFER_SIZE){
        printf("[thread %d] Transferred file (%d bytes)\n", thread_id, size);
        return("ACK");
    } else {
        fd = open(path, O_APPEND|O_WRONLY, S_IRUSR|S_IWUSR); 

        int i = 1;
        int bytes_to_write;
        int bytes_remaining = size - 1024;;
        if (bytes_remaining > 1024) bytes_to_write = 1024;
        else bytes_to_write = bytes_remaining;
        
        while (data[i]){
            n = write(fd, data[i], bytes_to_write);
            bytes_remaining -= bytes_to_write;
            if (n < 0){
                perror("write()");
                return("ERROR");
            }
            i++;
        }
    }

    printf("[thread %d] Transferred file (%d bytes)\n", thread_id, size);
    return("ACK");
}

/*
* Overwrites file in .storage directory.
* If the file doesn't exist, it is not created.
* Returns success or error code.
* Args:
*   size: number of bytes to write
*   filename: name of file to write to
*   data: string array containing data to write
*   thread_id: id# of thread that called the function
*/
char* updateFile(int size, char* filename, char** data, int thread_id){
    //Construct the relative path to the file to create
    char path[80] = "./.storage/";
    strcat(path,filename);

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
        int bytes_to_write;
        if (size > 1024) bytes_to_write = 1024;
        else bytes_to_write = size;

        n = write(fd, data[0], bytes_to_write);

        if (n < 0){
            perror("write()");
            return("ERROR");
        }
    }
    close(fd);
    
    if(size < BUFFER_SIZE){
        printf("[thread %d] Transferred file (%d bytes)\n", thread_id, size);
        return("ACK");
    } else {
        fd = open(path, O_APPEND|O_WRONLY, S_IRUSR|S_IWUSR); 

        int i = 1;
        int bytes_to_write;
        int bytes_remaining = size - 1024;;
        if (bytes_remaining > 1024) bytes_to_write = 1024;
        else bytes_to_write = bytes_remaining;
        
        while (data[i]){
            n = write(fd, data[i], bytes_to_write);
            bytes_remaining -= bytes_to_write;
            if (n < 0){
                perror("write()");
                return("ERROR");
            }
            i++;
        }
    }

    printf("[thread %d] Transferred file (%d bytes)\n", thread_id, size);
    return("ACK");
}

/*
* Reads contents of file in .storage directory and stores in buffer.
* Length of read data is stored in len.
* Returns success or error code.
* Args:
*   cmd: args of command separated into array
*   argc: number of arguments in cmd[]
*   buffer: buffer to fill with read data
*   len: number of bytes read into buffer (as a string)
*   thread_id: id# of thread that called the function
*/
char* readFile(char *cmd[], int argc, char* buffer, char* len, int thread_id){
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

    printf("[thread %d] Transferred file (%d bytes)\n", thread_id, n);
    return("ACK");
}