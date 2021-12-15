#ifndef __CHAP__
#define __CHAP__
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "dummy_ftp.h"



int chap_req(size_t type,const char* data,size_t len,
                      size_t seq = 0,size_t cmd = 0,const char * option = NULL);
int chap_auth(size_t type,const char* data,size_t len,
                      size_t seq = 0,size_t cmd = 0,const char * option = NULL);
char chap_reply(const char* recv_data,size_t type,const char* data,size_t len,
                      size_t seq = 0,size_t cmd = 0,const char * option = NULL);
int chap_result(const char* recv_data,size_t type,const char* data,size_t len,
                      size_t seq = 0,size_t cmd = 0,const char * option = NULL);


#endif