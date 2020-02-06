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
bool sock=false;
int num=10;
int range=1000000;
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

    int tmp;
    srand((uint64_t)time(0));
    for(int i=0;i<num;i++){
        tmp=rand()%range;
        std::sprintf(key,"%d",tmp);
        value=memcached_get(memc,key,std::strlen(key), &value_length, &flags, &error);
        if(error==memcached_return_t::MEMCACHED_SUCCESS){
            printf("%d:%s\n",tmp,value);
        }else{
            printf("%d:error\n",tmp);
        }
    }
    return 0;
}

