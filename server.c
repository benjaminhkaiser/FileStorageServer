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
*/
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

    return("ACK");
}

/*
* Overwrites file in .storage directory.
* If the file doesn't exist, it is not created.
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

    return("ACK");
}

/*
* Reads contents of file in .storage directory and stores in buffer.
*/
char* readFile(char *cmd[], int argc, char* buffer){
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

    //Otherwise, open the file and read from it
    int fd = open(path, O_RDONLY);
    if (fd < 0){
        perror("open()");
        return("ERROR");
    } else {
        char file_contents[80];
        int n = read(fd,file_contents,80);
        if (n < 0){
            perror("read()");
            return("ERROR");
        }
        strcpy(buffer,file_contents);
    }

    return("ACK");
}

int main(int argc, char *argv[])
{
    unsigned short port = 0;    //port number to connect to
    int n = 0;
    
    //get port from arguments
    if (argc == 2){
        port = (unsigned short) strtoul(argv[1], NULL, 0);    //convert to numeric value
        if (!(port >= 8000 && port <= 9000)){
            //invalid port number
            printf("\n Usage: %s <port (8000-9000)> \n",argv[0]);
            exit(1);
        }
    } else {
        //invalid number of arguments
        printf("\n Usage: %s <port (8000-9000)> \n",argv[0]);
        exit(1);
    }

    int sock;   //socket file descriptor
    int newsock = 0;    //new socket file descriptor
    struct sockaddr_in server; //socket struct from socket.h
    //struct sockaddr_in client; //socket struct from socket.h

    char buffer[BUFFER_SIZE];   //buffer to hold rec'd messages

    //open socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        perror("socket()");
        exit(1);
    }

    //set server data
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port); 

    //bind server to socket
    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0){
        perror ("bind()");
        exit(1);
    }

    //listen to bound socket
    listen(sock, 5); 
    printf( "Listener socket created and bound to port %d\n", port );

    while(1)
    {

        //wait for client connection
        printf("Blocked on accept()\n");
        newsock = accept(sock, (struct sockaddr*)NULL, NULL); 
        printf( "Accepted client connection\n" );
        fflush( NULL );

        //get message from client
        n = recv( newsock, buffer, BUFFER_SIZE - 1, 0 );
        if ( n < 1 ) {
            perror( "recv()" );
        } else {
            //finalize buffer and print
            buffer[n] = '\0';
            printf( "Rcvd: %s\n", buffer );

            //initialize array to hold parsed command
            char** cmd = malloc(80);
            int i;
            for (i = 0; i != 80; i++){
                cmd[i] = malloc(80*sizeof(char));
            }

            parseCommand(buffer, cmd);

            //find end of cmd array
            i = 0;
            while (cmd[i]){
                //printf("%d: %s\n", i, cmd[i]);
                i++;
            }
            

            //execute command
            //TODO: return success/failure message to client
            if (strcmp(cmd[0],"ADD") == 0){
                addFile(cmd,i);
            } else if (strcmp(cmd[0],"UPDATE") == 0){
                updateFile(cmd,i);
            } else if (strcmp(cmd[0],"READ") == 0){
                readFile(cmd,i,buffer);
                printf("%s\n",buffer);
            } else {
                printf("Invalid command: %s\n", buffer);
            }           
        }

        close(newsock);
        sleep(1);
     }
}