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
#define TEST_NUM 1000000
#define PACKAGE_NUM 1
#define MEMCACHED_MAX_BUF 8192
#define THREAD_NUM 1
#define ST_LEN 100
#define sock true

using namespace std;

char *database[TEST_NUM];

void con_database();

int con_send_package(int num,char * st_buf);



void *thread_send(){
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

    for(int i=0;i<TEST_NUM;i++){
        write(connect_fd,database[i],48);
    }

    close(connect_fd);

}

int main(void)
{

    con_database();
    memcached_server_st *servers;
    if(sock){
        servers = memcached_servers_parse(UNIX_DOMAIN);
    }else{
        servers = memcached_servers_parse("localhost");
    }
    memcached_st * memc = memcached_create(NULL);
    memcached_server_push(memc, servers);
    //memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, (uint64_t) 1);

    char *value;
    int value_length;
    unsigned int flags;
    memcached_return_t error;
    unsigned  long getkey=123;
    value=memcached_get(memc, (const char *)&getkey, 8, reinterpret_cast<size_t *>(&value_length), &flags, &error);
    printf("send start get\n");

    Tracer tracer;
    tracer.startTime();
    pthread_t pid[THREAD_NUM];

    for(int i=0;i<THREAD_NUM;i++){
        if(pthread_create(&pid[i], NULL, reinterpret_cast<void *(*)(void *)>(thread_send), NULL) != 0){
            printf("create pthread error\n");
            return 0;
        }
    }
    for(int i=0;i<THREAD_NUM;i++){
        pthread_join(pid[i],NULL);
    }
    printf("%lu\n",tracer.getRunTime());

    getkey=321;
    value=memcached_get(memc, (const char *)&getkey, 8, reinterpret_cast<size_t *>(&value_length), &flags, &error);
    printf("send finish get\n");
    memcached_free(memc);

    return 0;
}


void con_database(){
    char *tmp_buf;
    for(int i=0;i<TEST_NUM;i++){
        tmp_buf=(char *)malloc(200);
        memset(tmp_buf,0,200);
        con_send_package(i,tmp_buf);
        database[i]=tmp_buf;
    }

}

int con_send_package(int num,char * st_buf){
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
