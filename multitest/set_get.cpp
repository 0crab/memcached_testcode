/*
Binary Protocol Reference:
 https://github.com/memcached/memcached/wiki/BinaryProtocolRevamped
 */
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <cstring>
#include <pthread.h>
#include "../tracer.h"
#include <libmemcached/memcached.h>
#define UNIX_DOMAIN "/home/czl/memcached.sock"
#define SET_NUM 1000000
#define GET_NUM 1000000
#define PACKAGE_NUM 1
#define MEMCACHED_MAX_BUF 8192

#define SET_THREAD_NUM 1
#define GET_THREAD_NUM 0
#define ST_LEN 100
#define sock true

using namespace std;

char *database[SET_NUM];

void con_database();

//memset before pass in st_buf
int con_set_package(int num,char * st_buf);
int con_get_package(int num,char * st_buf);


void *thread_get_send(){
    unsigned int connect_fd;
    static struct sockaddr_un srv_addr;
    //create unix socket
    connect_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(connect_fd < 0) {
        perror("cannot create communication socket");
        return NULL;
    }
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path,UNIX_DOMAIN);
    //connect server;
    if( connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) == -1) {
        perror("cannot connect to the server");
        close(connect_fd);
        return NULL;
    }

    char *st_buf=(char *)malloc(100);
    char *rec_buf=(char *)malloc(100);
    memset(st_buf,0,100);
    memset(rec_buf,0,100);
    int package_len;
    for(int i=0;i<GET_NUM;i++){
        package_len=con_get_package(i,st_buf);
        write(connect_fd,st_buf,package_len);
        read(connect_fd,rec_buf,100);
        //printf("%s\n",rec_buf);
    }
    close(connect_fd);
}


void *thread_set_send(){
    unsigned int connect_fd;
    static struct sockaddr_un srv_addr;
    //create unix socket
    connect_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(connect_fd < 0) {
        perror("cannot create communication socket");
        return NULL;
    }
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path,UNIX_DOMAIN);
    //connect server;
    if( connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) == -1) {
        perror("cannot connect to the server");
        close(connect_fd);
        return NULL;
    }


    for(int i=0;i<SET_NUM;i++){
        write(connect_fd,database[i],48);

    }
    close(connect_fd);

}

int main(void)
{

    con_database();
    pthread_t set_pid[SET_THREAD_NUM];
    pthread_t get_pid[GET_THREAD_NUM];

    Tracer tracer;
    tracer.startTime();

    for(int i=0;i<SET_THREAD_NUM;i++){
        if(pthread_create(&set_pid[i], NULL, reinterpret_cast<void *(*)(void *)>(thread_set_send), NULL) != 0){
            printf("create pthread error\n");
            return 0;
        }
    }

    for(int i=0;i<GET_THREAD_NUM;i++){
        if(pthread_create(&get_pid[i], NULL, reinterpret_cast<void *(*)(void *)>(thread_get_send), NULL) != 0){
            printf("create pthread error\n");
            return 0;
        }
    }

    for(int i=0;i<SET_THREAD_NUM;i++){
        pthread_join(set_pid[i],NULL);
    }

    for(int i=0;i<GET_THREAD_NUM;i++){
        pthread_join(get_pid[i],NULL);
    }


    printf("%lu\n",tracer.getRunTime());



    return 0;
}


void con_database(){
    char *tmp_buf;
    for(int i=0;i<SET_NUM;i++){
        tmp_buf=(char *)malloc(200);
        memset(tmp_buf,0,200);
        con_set_package(i,tmp_buf);
        database[i]=tmp_buf;
    }

}

int con_set_package(int num,char * st_buf){
    //Magic: Request
    st_buf[0]=0x80;
    //Opcode: SetQ
    st_buf[1]=0x11;
    //Extra length
    st_buf[4]=0x08;

    //key_len=8,
    //value_len=8
    //total_body=24 only use a single byte
    st_buf[3]=8;
    st_buf[11]=24;

    unsigned long *keyptr=(unsigned long *)(st_buf+32);
    *keyptr=(unsigned long )num;

    char value_buf[8];
    char num_buf[8];
    int numlen;
    memset(value_buf,0x2a, sizeof(value_buf));
    sprintf(num_buf,"%d",num);
    numlen=strlen(num_buf);
    if(numlen>8) printf("num too long \n");
    memcpy(value_buf,num_buf,numlen);
    memcpy(st_buf+40,value_buf,8);
    return 48;
}

int con_get_package(int num,char * st_buf){
    //Magic: Request
    st_buf[0]=0x80;
    //keylength
    st_buf[3]=0x08;
    st_buf[11]=0x08;

    unsigned long *keyptr=(unsigned long *)(st_buf+24);
    *keyptr=(unsigned long )num;

    return 32;
}
