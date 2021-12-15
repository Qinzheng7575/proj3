#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "dummy_ftp.h"
//#include "CHAP.h"


char res = 0;   //全局变量储存运算结果

#define SECRET_KEY 12345    //定义CHAP密钥

void visit(char brr[],int length){
	int i;
	for(i=0;i<length;i++){
		printf("%d",brr[i]);
		printf("  ");
	}
}

int chap_req(size_t type ,const char* data,size_t len,
                      size_t seq ,size_t cmd ,const char * option ){
    //mysend(2, NULL, 0, 0);
	
}

int chap_auth(size_t type,const char* data,size_t len,
                      size_t seq ,size_t cmd,const char * option){
    int N;
    srand((unsigned int)time(0));	//初始化种子为随机值
	  N=rand()%10+1;
	  char rand_list [12];     //存储随机数组
	
    rand_list[0] = N;   //第一个数储存数据长度
	for(int i = 1; i <= N; i++){
		rand_list[i] = rand()%101+1;
	}
	printf("数组长度为:%d\n",N);
	visit(rand_list,strlen(rand_list));
    
	printf("\n");
	printf("长度%ld\n",strlen(rand_list));

    for(int i = 1; i <= strlen(rand_list); i++){
        res = rand_list[i] + res;
    }
    printf("运算结果：%d\n",res); 
    res = res^SECRET_KEY;
    printf("运算结果：%d\n",res);
    // free(p);
    //mysend(2, rand_list, strlen(rand_list), 1);
    return 0;
}

//先通过handle_data函数获取data字段后传入recv_data进行计算
char chap_reply(const char* recv_data,size_t type,const char* data,size_t len,
                      size_t seq ,size_t cmd ,const char * option){
	char re_res [1];
    for(int i = 1; i <= recv_data[0]; i++){
		re_res[0] = re_res[0] + recv_data[i];
		
	}
	re_res[0] = re_res[0]^SECRET_KEY;
	//mysend(2, re_res, strlen(re_res), 2);
    return re_res[0];
}


int chap_result(const char* recv_data,size_t type,const char* data,size_t len,
                      size_t seq ,size_t cmd,const char * option ){
	char answer = recv_data[0];
	//printf("%d\n%d",answer,res);
	if(answer == res){
		printf("认证成功\n");
		mysend(2, NULL, 0, 3);
		return 1;
	}
	else{
		printf("认证失败\n");
		mysend(2, NULL, 0, 4);
		return 0;
	}  
}


