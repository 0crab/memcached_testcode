#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <cstring>
#include "tracer.h"
#define UNIX_DOMAIN "/home/iclab/czl/memcached.sock"
#define TEST_NUM 1000000
#define PACKAGE_NUM 1
#define MEMCACHED_MAX_BUF 8192
#define ST_LEN 100
using namespace std;

char send_buf[MEMCACHED_MAX_BUF];


int main(void)
{
	unsigned int connect_fd;
	unsigned int  ret;
	static struct sockaddr_un srv_addr;
	//create unix socket
	connect_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(connect_fd < 0) {
		perror("cannot create communication socket");
		return 1;
	}
	srv_addr.sun_family = AF_UNIX;
	strcpy(srv_addr.sun_path,UNIX_DOMAIN);
	//connect server
	ret = connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	if(ret == -1) {
		perror("cannot connect to the server");	
		close(connect_fd);
		return 1;
	}

	int num=0;
	int total_len,st_len,num_len,num_len_len;
	//string st;
	char st_buf[ST_LEN];
	char tmp1[5]="set ";
	char tmp2[6]=" 0 0 ";
	char tmp3[16]=" noreply\r\nhello";
	char tmp4[3]="\r\n";
	char tmp_num[8];
	char tmp_num_len[3];
	//cmd number =TEST_NUM*PACKAGE_NUM
	Tracer tracer;
	long runtime=0;
	
	for(int i=0;i<TEST_NUM;i++){
		total_len=0;
		memset(send_buf,0,sizeof(send_buf));
		for(int j=0;j<PACKAGE_NUM;j++){
			num=i*PACKAGE_NUM+j;

			sprintf(tmp_num,"%d",num);
			num_len=strlen(tmp_num);
			sprintf(tmp_num_len,"%d",num_len+5);
			num_len_len=strlen(tmp_num_len);

			memset(st_buf,0,sizeof(st_buf));
			memcpy(st_buf,tmp1,4);
			memcpy(st_buf+4,tmp_num,num_len);
			memcpy(st_buf+4+num_len,tmp2,5);
			memcpy(st_buf+9+num_len,tmp_num_len,num_len_len);
			memcpy(st_buf+9+num_len+num_len_len,tmp3,15);
			memcpy(st_buf+24+num_len+num_len_len,tmp_num,num_len);
			memcpy(st_buf+24+2*num_len+num_len_len,tmp4,2);
			/*
					
			
			for(int k=0;k<st_len;k++){
				printf("%d:%d %c\n",k,st_buf[k],st_buf[k]);
			}
			*/
			st_len=26+2*num_len+num_len_len;
			memcpy(send_buf+total_len,st_buf,st_len);
			total_len+=st_len;
			
		}
		tracer.startTime();
		//printf("%s\n",st_buf);	
		//write(connect_fd,st_buf,st_len);
		write(connect_fd,send_buf,total_len);
		runtime+=tracer.getRunTime();
	}
	
	printf("%ld\n",runtime);
	printf("p:%d\tp_num:%d\ttotal:%d\n",PACKAGE_NUM,TEST_NUM,PACKAGE_NUM*TEST_NUM);
	
	
	close(connect_fd);
	return 0;
}
