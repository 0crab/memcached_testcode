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
#define TEST_NUM 100
#define PACKAGE_NUM 10
#define MEMCACHED_MAX_BUF 8192
#define ST_LEN 100
using namespace std;

char *database[TEST_NUM];

void con_database();

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

    con_database();
    char read_buf[4096];
    int readnum;
    memset(read_buf,0, sizeof(read_buf));
    Tracer tracer;
    long runtime=0;
    for(int i=0;i<TEST_NUM/PACKAGE_NUM;i++){
        tracer.startTime();
        int sendlen=strlen(database[i]);
        write(connect_fd,database[i],sendlen);
        readnum=read(connect_fd,read_buf,4096);
        while(readnum>=4096){
            printf("%s",read_buf);
            readnum=read(connect_fd,read_buf,4096);
        }
        printf("%s",read_buf);
        runtime+=tracer.getRunTime();
    }
    printf("%ld\n",runtime);
    close(connect_fd);
    return 0;
}

void con_database(){
    char * st_buf;
    for(int i=0;i<TEST_NUM/PACKAGE_NUM;i++){
        st_buf= static_cast<char *>(malloc(4096));
        memset(st_buf,0, sizeof(st_buf));
        con_send_package(i*PACKAGE_NUM,st_buf);
        database[i]=st_buf;
    }
}

int con_send_package(int num,char * st_buf){
    char getstr[10]="get ";
    char numbuf[10];
    memcpy(st_buf,getstr,4);
    int offset=4;
    int numlen;
    for(int i=0;i<PACKAGE_NUM;i++){
        sprintf(numbuf,"%d",num+i);
        numlen=strlen(numbuf);
        memcpy(st_buf+offset,numbuf,numlen);
        offset+=numlen;
        st_buf[offset++]=' ';
    }
    st_buf[offset]='\r';
    st_buf[offset+1]='\n';

}
