#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int sock = 0, n = 0;
    char ip[10] = {"127.0.0.1"};
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serv_addr; 

    if(argc != 2)
    {
        printf("\n Usage: %s <port> \n",argv[0]);
        return 1;
    } 

    memset(buffer, '0',sizeof(buffer));
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    unsigned short port = (unsigned short) strtoul(argv[1], NULL, 0); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); 

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    if( connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 

    //-----COMMANDS-----
    char* cmd = "READ blah.txt";
    n = write( sock, cmd, strlen( cmd ) );
    if ( n < strlen( cmd ) ) {
        perror( "write()" );
        exit( 1 );
    }
    sleep(5);

    //TODO: Multiple commands like this not working
    /*
    cmd = "hello again";
    n = write( sock, cmd, strlen( cmd ) );
    if ( n < strlen( cmd ) ) {
        perror( "write()" );
        exit( 1 );
    }
    sleep(5);
    */
    return 0;
}