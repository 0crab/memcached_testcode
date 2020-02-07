//
// Created by czl on 2/5/20.
//
//
// Created by Michael on 10/27/19.
//
#include <iostream>
#include <cstring>
#include <libmemcached/memcached.h>
#include <stdlib.h>
#include "tracer.h"
bool sock=true;
int num=1000000;
int main(int argc, char **argv) {
    memcached_server_st *servers;
    if(sock){
         servers = memcached_servers_parse("/home/czl/memcached.sock");
    }else{
         servers = memcached_servers_parse("localhost");
    }
    memcached_st * memc = memcached_create(NULL);
    memcached_server_push(memc, servers);

    char key[64];
    char *value;
    size_t value_length;
    uint32_t flags;
    memcached_return_t error;

    Tracer tracer;
    tracer.startTime();
    int success_num=0,fail_num=0;
    for(int i=0;i<num;i++){
        std::sprintf(key,"%d",i);
        value=memcached_get(memc,key,std::strlen(key), &value_length, &flags, &error);
        if(error==memcached_return_t::MEMCACHED_SUCCESS){
            //printf("%d:%s\n",tmp,value);
            success_num++;
        }else{
            //printf("%d:error\n",tmp);
            fail_num++;
        }
    }
    printf("%ld\n",tracer.getRunTime());
    printf("total get:%d\nsuccess:%d\nfail:%d\n",num,success_num,fail_num);
    return 0;
}

