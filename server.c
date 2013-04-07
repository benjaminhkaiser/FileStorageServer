#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    unsigned short port = 0;    //port number to connect to

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

    char buffer[BUFFER_SIZE];
    time_t ticks; 

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

        ticks = time(NULL);
        snprintf(buffer, sizeof(buffer), "%.24s\r\n", ctime(&ticks));
        write(newsock, buffer, strlen(buffer)); 

        close(newsock);
        sleep(1);
     }
}