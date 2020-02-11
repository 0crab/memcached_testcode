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
#define MSG_IN_PACKAGE 100
#define MEMCACHED_MAX_BUF 8192
#define ST_LEN 40
using namespace std;

char send_buf[MEMCACHED_MAX_BUF];

unsigned  int connect_to_server();

int main(void)
{
    unsigned int connect_fd;
    connect_fd=connect_to_server();
    if(connect_fd==-1) return 1;

    int num=0;
    int total_len,st_len,num_len;
    char st_buf[ST_LEN];
    char tmp1[5]="set ";
    char tmp2[21]=" 0 0 10 noreply\r\nmsg";
    char tmp3[3]="\r\n";
    char tmp_num[8];

    Tracer tracer;
    long runtime=0;

    //i:package num
    for(int i=0;i<TEST_NUM/MSG_IN_PACKAGE;i++){
        total_len=0;
        memset(send_buf,0,sizeof(send_buf));
        for(int j=0;j<MSG_IN_PACKAGE;j++){
            num=i*MSG_IN_PACKAGE+j;

            sprintf(tmp_num,"%d",num);
            num_len=strlen(tmp_num);


            memset(st_buf,0,sizeof(st_buf));

            memcpy(st_buf,tmp1,4);
            memcpy(st_buf+4,tmp_num,num_len);
            memcpy(st_buf+11,tmp2,20);
            memcpy(st_buf+31,tmp_num,num_len);
            memcpy(st_buf+38,tmp3,2);
            //fill ' ' and '*' in the gap
            memset(st_buf+4+num_len,' ',7-num_len);
            memset(st_buf+31+num_len,'*',7-num_len);

            memcpy(send_buf+total_len,st_buf,ST_LEN);
            total_len+=ST_LEN;

        }
        tracer.startTime();
        write(connect_fd,send_buf,total_len);
        runtime+=tracer.getRunTime();
    }

    printf("%ld\n",runtime);
    printf("p:%d\tp_num:%d\ttotal:%d\n",MSG_IN_PACKAGE,TEST_NUM/MSG_IN_PACKAGE,TEST_NUM);
    close(connect_fd);
    return 0;
}
unsigned  int connect_to_server(){
    unsigned int connect_fd;
    unsigned int  ret;
    static struct sockaddr_un srv_addr;
    //create unix socket
    connect_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(connect_fd < 0) {
        perror("cannot create communication socket");
        return -1;
    }
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path,UNIX_DOMAIN);
    //connect server
    ret = connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if(ret == -1) {
        perror("cannot connect to the server");
        close(connect_fd);
        return -1;
    }
    return connect_fd;
}

