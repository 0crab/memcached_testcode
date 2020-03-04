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
#define UNIX_DOMAIN "/home/czl/memcached.sock"
#define TEST_NUM 1000000
#define PACKAGE_NUM 1
#define MEMCACHED_MAX_BUF 8192
#define THREAD_NUM 1
#define ST_LEN 100
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

    int package_len;
    unsigned long tmpkey;
    for(int i=0;i<TEST_NUM;i++){

        package_len=0;
        tmpkey=(unsigned long)*(unsigned long *)(database[i]+32);

        while(1){
            if(tmpkey==0){
                break;
            }
            else{
                package_len++;
            }
            tmpkey /= 10;
        }
        package_len=package_len==0?46:package_len+45;

        write(connect_fd,database[i],package_len);
    }

    close(connect_fd);

}

int main(void)
{
    con_database();

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

    char key_buf[10];
    memset(key_buf,0, sizeof(key_buf));
    char value_buf[20]={'h','e','l','l','o'};
    unsigned char key_len;
    unsigned char total_body;

    sprintf(key_buf,"%d",num);
    key_len=strlen(key_buf);
    memcpy(value_buf+5,key_buf,key_len);
    total_body=8+5+8+key_len;

    //key_len<255,total_body<255 only use a single byte
    st_buf[3]=8;
    st_buf[11]=total_body;

    unsigned long *keyptr=(unsigned long *)(st_buf+32);
    *keyptr=(unsigned long )num;
    memcpy(st_buf+32+8,value_buf,5+key_len);

    return 45+key_len;
}
