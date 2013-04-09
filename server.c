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

#define BUFFER_SIZE 1024    //TODO: fix for arbitrary length message
extern int errno;

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
            
            //execute command and return success/failure message to client
            if (strcmp(cmd[0],"ADD") == 0){
                char* msg = addFile(cmd,i);
                n = send(newsock, msg, strlen(msg), 0);
                if ( n < strlen( msg ) ) {
                    perror( "Write()" );
                } else {
                    printf("Sent: %s\n", msg);
                }
                close( newsock );
            } else if (strcmp(cmd[0],"UPDATE") == 0){
                char* msg = updateFile(cmd,i);
                n = send(newsock, msg, strlen(msg), 0);
                if ( n < strlen( msg ) ) {
                    perror( "Write()" );
                } else {
                    printf("Sent: %s\n", msg);
                }
                close( newsock );
            } else if (strcmp(cmd[0],"READ") == 0){
                char len[10];
                len[0] = '\0';

                char msg[BUFFER_SIZE];
                msg[0] = '\0';

                char* ret = readFile(cmd,i,buffer,len);

                strcat(msg, ret);
                strcat(msg, " ");
                strcat(msg, len);
                strcat(msg, "\n");
                strcat(msg,buffer);

                n = send(newsock, msg, strlen(msg), 0);
                if ( n < strlen( msg ) ) {
                    perror( "Write()" );
                } else {
                    printf("Sent: %s\n", msg);
                }
                close( newsock );
            } else {
                //TODO: this doesn't work
                char msg[BUFFER_SIZE] = "Invalid command: ";
                strcat(msg, buffer);
                n = send(newsock, msg, strlen(msg), 0);
                if ( n < strlen( msg ) ) {
                    perror( "Write()" );
                }
                close( newsock );
            }           
        }

        close(newsock);
        sleep(1);
     }
}