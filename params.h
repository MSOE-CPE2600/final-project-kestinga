/**********************************
* Filename: params.h
* Description: final project thread params
* Author: Abe Kesting
* Date: 12/4/24
***********************************/


struct params {
    int sock, *quit, is_host, fd;
    pthread_mutex_t *mutex;
};