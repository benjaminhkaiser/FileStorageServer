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
* maybe implement forking on client connection like in class?
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
    printf("1\n");
    char buffer[BUFFER_SIZE];   //buffer to hold rec'd messages
    struct arg_struct *args = temp;
   
    int newsock = args->arg1;
    int t = args->arg2;

    printf( "Received incoming connection from <client-hostname>\n" );
    fflush( NULL );

    int n = 1;  //return val for recv

    char cmd[100];   //first part of command (cmd, size, and filename)
    int data_flag = 0;  //flag to tell if in data segment of cmd or not
    int i = 0;  //counter for location in data[] array
    char** data = (char**) calloc(BUFFER_SIZE*BUFFER_SIZE, sizeof(char*)); //string array to hold rec'd data
    data[0] = malloc(1024*sizeof(char));    //initialize first record in array
    

    //while recv succeeds, get one char at a time:
    //  if not in the data section, check to see if thar char is \n
    //      if it isn't, append new char to cmd
    //      if it is, set the data_flag to true
    //  once we're in the data section, all chars get added to the data[] array
    //      each slot is 1024 chars. first we check if the slot is full. if it is,
    //      we increment the counter (i) and allocate memory to the new slot.
    //      then we append the char to the [i] slot.

    while (n > 0){
        n = recv(newsock, buffer, 1, 0);
        if (n > 0){
            buffer[n] = '\0';
            if (data_flag){
                if(strlen(data[i]) == BUFFER_SIZE-1){
                    i++;
                    data[i] = malloc(1024*sizeof(char));
                }
                strcat(data[i],buffer);
            } else if (strcmp(buffer, "\n") == 0){
                data_flag = 1;
            } else {
                strcat(cmd,buffer);
            }
        } else if (n < -1){
            perror( "recv()" );
            exit(1);
        }
    }

    //Print receipt message
    printf( "[thread %d] Rcvd: ", t);
    i = 0;
    printf("%s\n", cmd);
    while (data[i]){
        printf("%s",data[i]);
        i++;
    }

    if(prefixMatch(cmd,"ADD")){
        char* filename = malloc(80*sizeof(char));
        parseFilename(cmd, filename);
        int size = parseSize(cmd);
        char* msg = addFile(size, filename, data, t);

        n = send(newsock, msg, strlen(msg), 0);
        if ( n < strlen( msg ) ) {
            perror( "send()" );
        } else {
            printf("[thread %d] Sent: %s\n", t, msg);
        }
    } else if (prefixMatch(cmd, "UPDATE")){
        char* filename = malloc(80*sizeof(char));
        parseFilename(cmd, filename);
        int size = parseSize(cmd);
        char* msg = updateFile(size, filename, data, t);

        n = send(newsock, msg, strlen(msg), 0);
        if ( n < strlen( msg ) ) {
            perror( "send()" );
        } else {
            printf("[thread %d] Sent: %s\n", t, msg);
        }
    } else if (prefixMatch(cmd, "READ")){
        char* filename = malloc(80*sizeof(char));
        parseFilename(cmd, filename);
        int bytes = readFile(filename, data, t);

        char* msg = malloc(1024*1024*50*sizeof(int));
        if (bytes == -1){
            msg = "NO SUCH FILE";
        } else if (bytes == -2){
            msg = "ERROR";
        } else {
            msg = "ACK";
            strcat(msg, " ");
            char bytesstr[15];
            sprintf(bytesstr,"%d",bytes);
            strcat(msg, bytesstr);
            strcat(msg, "\n");
            //figure out how to send actual data
        }


    } else {
        char msg[BUFFER_SIZE] = "Invalid command: ";
        strcat(msg, cmd);
        n = send(newsock, msg, strlen(msg), 0);
        if ( n < strlen( msg ) ) {
            perror( "send()" );
        }  else {
            printf("[thread %d] Sent: %s\n", t, msg);
        }
    }

    close(newsock);
    return NULL;
}

int main(int argc, char *argv[]){
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

        //set up arguments for thread creation
        struct arg_struct args;
        args.arg1 = newsock;
        args.arg2 = t;

        printf("0\n");

        //create thread to handle client processing
        int rc = pthread_create(&threads[t], NULL, processClient, (void *)&args);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }

        sleep(1);
     }

    return(0);
}