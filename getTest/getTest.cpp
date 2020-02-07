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
int num=1000000;
bool sock=true;
bool inbatch=true;
int batchlen=100;

int main(int argc, char **argv) {
    memcached_server_st *servers;
    if(sock){
         servers = memcached_servers_parse("/home/czl/memcached.sock");
    }else{
         servers = memcached_servers_parse("localhost");
    }
    memcached_st * memc = memcached_create(NULL);
    memcached_server_push(memc, servers);

    int success_num=0,fail_num=0;
    Tracer tracer;
    if(!inbatch){
        char key[64];
        char *value;
        size_t value_length;
        uint32_t flags;
        memcached_return_t error;
        tracer.startTime();
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
    }else{
        memcached_return_t rc;

        size_t key_length[100];
        char *keys[100];
        char *tmpkey;
        size_t tmpkeylen;
        int i=0;
        Tracer tracer;
        unsigned long runtime=0;

        while(i<num){
            for(int j=0;j<batchlen&&j<num;j++){
              tmpkey=(char *)malloc(10);
              std::sprintf(tmpkey,"%d",i);
              tmpkeylen=std::strlen(tmpkey);
              keys[j]=tmpkey;
              key_length[j]=tmpkeylen;
              i++;
            }
            tracer.startTime();
            rc= memcached_mget(memc, keys, key_length,batchlen);
            runtime+=tracer.getRunTime();
        }
        printf("%lu\n",runtime);

        unsigned int x;
        uint32_t flags;
        char return_key[MEMCACHED_MAX_KEY];
        size_t return_key_length;
        char *return_value;
        size_t return_value_length;

        x= 0;
        while ((return_value= memcached_fetch(memc, return_key, &return_key_length,
                                              &return_value_length, &flags, &rc)))
        {
            //printf("%s\n",return_value);
            x++;
        }
        //printf("%d\n",x);
    }

    //printf("%ld\n",tracer.getRunTime());
    //printf("total get:%d\nsuccess:%d\nfail:%d\n",num,success_num,fail_num);
    return 0;
}

