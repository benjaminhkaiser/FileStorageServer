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

#define BUFFER_SIZE 1024
extern int errno;

/*
* Fills cmd[] with command separated into args.
*/
void parseCommand(char buffer[BUFFER_SIZE], char *cmd[]){
    //separate values into array
    cmd[0] = strsep(&buffer, " ");
    int i = 0;
    while (cmd[i]){
        i++;
        cmd[i] = strsep(&buffer, " ");
    }
}

int addFile(char *cmd[]){
    int n = mkdir(".storage", S_IRWXU);
    if (n < 0 && errno != EEXIST){
        perror("mkdir()");
        return(0);
    }

    return(1);
}

int updateFile(char *cmd[]){

    return(1);
}

int readFile(char *cmd[]){

    return(1);
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
    struct sockaddr_in client; //socket struct from socket.h

    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0){
        perror("socket()");
        exit(1);
    }

    memset(&server, '0', sizeof(server));
    memset(buffer, '0', sizeof(buffer)); 

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port); 

    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0){
        perror ("bind()");
        exit(1);
    }

    listen(sock, 5); 
    printf( "Listener socket created and bound to port %d\n", port );

    while(1)
    {
        printf("Blocked on accept()\n");
        newsock = accept(sock, (struct sockaddr*)NULL, NULL); 
        printf( "Accepted client connection\n" );
        fflush( NULL );

        n = recv( newsock, buffer, BUFFER_SIZE - 1, 0 );   // BLOCK
        if ( n < 1 ) {
            perror( "recv()" );
        }
        else {
            buffer[n] = '\0';
            printf( "Rcvd: %s\n", buffer );

            //initialize array
            char** cmd = malloc(80);
            int i;
            for (i = 0; i != 80; i++){
                cmd[i] = malloc(80*sizeof(char));
            }

            parseCommand(buffer, cmd);

            i = 0;
            while (cmd[i]){
                printf("%d: %s\n", i, cmd[i]);
                i++;
            }

            if (strcmp(cmd[0],"ADD") == 0){
                addFile(cmd);
            } else if (strcmp(cmd[0],"UPDATE") == 0){
                updateFile(cmd);
            } else if (strcmp(cmd[0],"READ") == 0){
                readFile(cmd);
            } else {
                printf("Invalid command: %s\n", buffer);
            }

            
            
        }

        close(newsock);
        sleep(1);
     }
}