/**********************************
* Filename: client.c
* Description: final project client. Modified Dr. Turney's code
* Author: Abe Kesting
* Course-Section: CPE 2600-111
* Date: 12/4/24
***********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "readthread.h"
#include "params.h"
// define some constants
#define MAX 256
#define SERVER_HOST "localhost"
#define SERVER_IP "172.17.50.196" //"127.0.0.1" $hostname -I
#define BUFLEN 256
#define SERVER_PORT 1234




int main (void) {
    struct sockaddr_in server_addr;
    int sock, r;
    char line[MAX];
    pthread_t thread;
    int quit = 0;

    printf("create stream socket\n");
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        perror("socket call failed");
        exit(1);
    }
    printf("fill server address with host IP and Port number\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    printf("connect to server\n");
    r = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (r<0) {
        perror("connect failed");
        exit(1);
    }
    printf("connected ok  ...\n");
    
    printf("client init done\n\n");

    struct params params; //set the params for the read thread
    params.sock = sock;
    params.quit = &quit;
    params.is_host = 0;
    params.fd = -1;
    params.mutex = NULL;
    pthread_create(&thread, NULL, read_thread, (void*)&params);
    printf("input lines to send\n");
    while (strcmp(line,"quit") && quit == 0) { // processing loop
        bzero(line, MAX);
        fgets(line, MAX, stdin);
        line[strlen(line)-1] = 0; 
        write(sock, line, MAX);
        printf("sent: %s\n\n", line); //send message to host
    }
    quit = 1;
    //clean up
    pthread_join(thread, NULL);
    close(sock);
    return 0;
}

