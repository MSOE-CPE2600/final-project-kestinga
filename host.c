/**********************************
* Filename: host.c
* Description: final project host, modified Dr. Turney's program 
* Author: Abe Kesting
* Course-Section: CPE 2600-111
* Date: 12/4/24
***********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>

#include "readthread.h"
#include "params.h"

// define some constants
#define MAX 256
#define SERVER_HOST "localhost"
#define SERVER_IP "172.17.50.196" //"127.0.0.1" $hostname -I
#define BUFLEN 256
#define SERVER_PORT 1234



int main (void) {
    char line[MAX];
    pthread_t thread;
    struct sockaddr_in server_addr, client_addr;
    int mysock, csock, r;
    unsigned int len;
    int quit = 0;

    printf("create stream socket\n");
    mysock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mysock < 0) {
        perror("socket call failed");
        exit(1);
    }
    printf("fill server address with host IP and Port number\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    printf("bind the socket");
    r = bind(mysock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (r<0) {
        perror("bind failed");
        exit(1);
    }
    printf("server is listening  ...\n");
    listen(mysock, 5); // queue length of 5
    printf("server init done\n");

    //Try to accept a client request
    printf("server: accepting new connection ...\n");
    len = sizeof(client_addr);
    csock = accept(mysock, (struct sockaddr *)&client_addr, &len);
    if (csock < 0) {
        perror("server accept error");
        exit(1);
    }
    
    printf("server accepted a client connection from \n");
    printf("Client: IP= %s port=%d \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    struct params params; //fill in parameters for read thread
    params.sock = csock;
    params.quit = &quit;
    params.is_host = 1;
    pthread_mutex_t temp = PTHREAD_MUTEX_INITIALIZER;
    params.mutex = &temp;
    pthread_mutex_init(params.mutex, NULL);

    struct timespec seconds; //get the time for the name of the log file
    clock_gettime(CLOCK_REALTIME, &seconds);

    struct tm *time = localtime(&(seconds.tv_sec));
    char open_time[255];
    sprintf(open_time, "%d-%d-%d %d:%2d:%2d", time->tm_mon+1, time->tm_mday, time->tm_year+1900, time->tm_hour, time->tm_min, time->tm_sec);
    char filename[264];
    sprintf(filename, "logs/%s.txt", open_time);
    params.fd = open(filename, O_CREAT | O_RDWR); //create the file with the name of the current time
    
    if (params.fd < 0) {
        perror("log error");
        exit(1);
    }

    chmod(filename, 0644); //set the permission flags to read/write
    char *anothermessage = "Chat opened ";
    write(params.fd, anothermessage, strlen(anothermessage));
    write(params.fd, open_time, strlen(open_time)); //write header of current time to log file and kick of thread

    //the thread reads from the other terminal and prints to both the log and the terminal
    pthread_create(&thread, NULL, read_thread, (void*)&params);

    printf("\n\ninput lines to send\n");
    char *message = "\nsent: ";
    
    while (strcmp(line,"quit") && quit == 0) { // processing loop
        bzero(line, MAX);
        fgets(line, MAX, stdin);
        line[strlen(line)-1] = 0; 
        write(csock, line, MAX); //send to client
        printf("sent: %s\n\n", line);

        //write to the log file
        pthread_mutex_lock(params.mutex);
        write(params.fd, message, strlen(message));
        write(params.fd, line, strlen(line));
        pthread_mutex_unlock(params.mutex);
    }
    quit = 1;
    pthread_join(thread, NULL);
    pthread_mutex_destroy(params.mutex);

    //prepare to write current time to log file
    struct timespec cseconds;
    clock_gettime(CLOCK_REALTIME, &cseconds);

    struct tm *ctime = localtime(&(cseconds.tv_sec));
    char close_time[255];
    sprintf(close_time, "\n\nChat closed %d-%d-%d %d:%2d:%2d", ctime->tm_mon+1, ctime->tm_mday, ctime->tm_year+1900, ctime->tm_hour, ctime->tm_min, ctime->tm_sec);
    write(params.fd, close_time, strlen(close_time));
    //formatted time written to log file
    close(csock);
    close(mysock);
    close(params.fd);
    //close sockets and log file descriptor
    return 0;
}