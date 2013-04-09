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
#include <pthread.h>
#include <fcntl.h>
#include "servercommands.h"

/*TODO LIST
* fix parseCommand() for arbitrary length message and messages w/spaces
*   -split by space until the first newline in buffer
*   -after newline, treat remainder as message
* free up completed threads
*   -make a thread struct that contains id and available flag
*   -when threads terminate, reset availability
* find client hostname and print upon connection
*/

#define BUFFER_SIZE 1024    
#define NUM_THREADS 1024

extern int errno;

/*
* Struct to hold arguments passed by pthread_create
*/
struct arg_struct {
    int arg1;               //newsock
    int arg2;               //t
};

/*
* Function called by pthread_create
* Receives data from socket and processes:
*   -Divides arguments into array
*   -Calls appropriate function based on first arg
*   -Sends success/error message back to client
*   -Closes socket
*/
void* processClient(void* temp){
    char buffer[BUFFER_SIZE];   //buffer to hold rec'd messages
    struct arg_struct *args = temp;
   
    int newsock = args->arg1;
    int t = args->arg2;

    printf( "Accepted client connection\n" );
    fflush( NULL );

    int n = recv(newsock, buffer, BUFFER_SIZE - 1, 0 );
    if ( n < 1 ) {
        perror( "recv()" );
    } else {
        //finalize buffer and print
        buffer[n] = '\0';
        printf( "[thread %d] Rcvd: %s\n", t, buffer );

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
            char* msg = addFile(cmd,i,t);
            n = send(newsock, msg, strlen(msg), 0);
            if ( n < strlen( msg ) ) {
                perror( "Write()" );
            } else {
                printf("[thread %d] Sent: %s\n", t, msg);
            }
            close( newsock );
        } else if (strcmp(cmd[0],"UPDATE") == 0){
            char* msg = updateFile(cmd,i,t);
            n = send(newsock, msg, strlen(msg), 0);
            if ( n < strlen( msg ) ) {
                perror( "Write()" );
            } else {
                printf("[thread %d] Sent: %s\n", t, msg);
            }
            close( newsock );
        } else if (strcmp(cmd[0],"READ") == 0){
            char len[10];
            len[0] = '\0';

            char msg[BUFFER_SIZE];
            msg[0] = '\0';

            char* ret = readFile(cmd,i,buffer,len,t);

            strcat(msg, ret);
            strcat(msg, " ");
            strcat(msg, len);
            strcat(msg, "\n");
            strcat(msg,buffer);

            n = send(newsock, msg, strlen(msg), 0);
            if ( n < strlen( msg ) ) {
                perror( "Write()" );
            } else {
                printf("[thread %d] Sent: %s\n", t, msg);
            }
            close( newsock );
        } else {
            char msg[BUFFER_SIZE] = "Invalid command: ";
            strcat(msg, buffer);
            n = send(newsock, msg, strlen(msg), 0);
            if ( n < strlen( msg ) ) {
                perror( "Write()" );
            }  else {
                printf("[thread %d] Sent: %s\n", t, msg);
            }
            close( newsock );
        }           
    }

    close(newsock);
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t threads[NUM_THREADS]; //array of available threads
    int sock;                       //socket file descriptor
    int newsock;                    //new socket file descriptor
    unsigned short port;            //port number to connect to

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

    struct sockaddr_in server; //socket struct from socket.h
    //struct sockaddr_in client; //socket struct from socket.h

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

    printf("Started file-server\n");

    //listen to bound socket
    listen(sock, 5); 
    printf( "Listening on port %d\n", port );

    int t = -1;

    while(1)
    {
        //get next available thread id
        if (t < NUM_THREADS) t++;
        else t = 0;

        //wait for client connection
        newsock = accept(sock, (struct sockaddr*)NULL, NULL); 
        struct arg_struct args;
        args.arg1 = newsock;
        args.arg2 = t;

        //create thread to handle client processing
        int rc = pthread_create(&threads[t], NULL, processClient, (void *)&args);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }

        sleep(1);
     }
}