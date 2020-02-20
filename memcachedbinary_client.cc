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
#include "tracer.h"
#define UNIX_DOMAIN "/home/czl/memcached.sock"
#define TEST_NUM 1000000
#define PACKAGE_NUM 1
#define MEMCACHED_MAX_BUF 8192
#define ST_LEN 100
using namespace std;

int con_send_package(int num,char * st_buf);

int main(void)
{
    unsigned int connect_fd;
    static struct sockaddr_un srv_addr;
    //create unix socket
    connect_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(connect_fd < 0) {
        perror("cannot create communication socket");
        return 1;
    }
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path,UNIX_DOMAIN);
    //connect server;
    if( connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) == -1) {
        perror("cannot connect to the server");
        close(connect_fd);
        return 1;
    }

    char send_buf[MEMCACHED_MAX_BUF];

    int package_len;
    Tracer tracer;
    long runtime=0;
    for(int i=0;i<TEST_NUM;i++){
        package_len=con_send_package(i,send_buf);
        tracer.startTime();
        write(connect_fd,send_buf,package_len);
        runtime+=tracer.getRunTime();
    }
    printf("%ld\n",runtime);
    close(connect_fd);
    return 0;
}

int con_send_package(int num,char * st_buf){
    //Magic: Request
    st_buf[0]=0x80;
    //Opcode: SetQ
    st_buf[1]=0x11;
    //Extra length
    st_buf[4]=0x08;

    char key_buf[10];
    char value_buf[20]={'h','e','l','l','o'};
    unsigned char key_len;
    unsigned char total_body;

    sprintf(key_buf,"%d",num);
    key_len=strlen(key_buf);
    memcpy(value_buf+5,key_buf,key_len);
    total_body=8+5+2*key_len;

    //key_len<255,total_body<255 only use a single byte
    st_buf[3]=key_len;
    st_buf[11]=total_body;

    memcpy(st_buf+32,key_buf,key_len);
    memcpy(st_buf+32+key_len,value_buf,5+key_len);

    return 37+2*key_len;
}
