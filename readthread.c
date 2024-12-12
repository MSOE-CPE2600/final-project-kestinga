/**********************************
* Filename: readthread.c
* Description: final project thread function and params
* Author: Abe Kesting
* Course-Section: CPE 2600-111
* Date: 12/4/24
***********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "params.h"


void *read_thread(void *param) {
    struct params *params = (struct params*)(param);
    char line[255] = {0};
    char *message = "\nrecieved: ";
    while(strcmp(line, "quit") != 0 && *(params->quit) == 0) {
        read(params->sock, line, 255); // read from other terminal
        if (strlen(line) > 0) {
            printf("recieved: %s\n\n", line);
            if (params->is_host) { //if it's the host, log the messages
                pthread_mutex_lock(params->mutex);
                write(params->fd, message, strlen(message));
                write(params->fd, line, strlen(line));
                pthread_mutex_unlock(params->mutex);
            }
        }

        
        if (strcmp(line, "quit") == 0) { //echo quit back, this makes the other read thread quit as well
            write(params->sock, line, 255);
        }
    }
    *(params->quit) = 1;
    return NULL;
}