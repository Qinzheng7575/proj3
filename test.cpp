#include <stdio.h>
#include<string.h>
#include "dummy_ftp.h"
#define data_len 10

FILE* Fid;
char data_buf[MAXN];//读取要发送文件的buf
static DFtpFrame sndbuf;
static CTRLFrame ctrlbuf;
int event = 0;



static int sw_send(const char* send_buf){
    printf("%s\n",send_buf);
    return(0);
}

static int get_data(const char* filename,const char* recv_buf,int len){//在handel_data之后出现
    Fid = fopen(filename,"a+");//因为是分块接收，使用a+参数不擦除原有内容
    fwrite(recv_buf,len,sizeof(char),Fid);
    
    fclose(Fid);
    return(0);

}

int check(){return(1);}

static inline void fill_data(size_t type,const char* data,size_t len,
                      size_t seq = 0,size_t cmd = 0,const char * option = NULL){
    len = len > MAXN ? MAXN : len;
    sndbuf.type = type;
    sndbuf.seq = seq;
    sndbuf.dlen = len;

    if (type == 0){
        ctrlbuf.cmd = cmd;
        memcpy(ctrlbuf.option, option, len-32);//填CTRL报文

        memcpy(sndbuf.data, &ctrlbuf, len);
    }
    else{
        memcpy(sndbuf.data, data, len);
    }
    sndbuf.check = check();
}





static inline int read_flie(const char* filename,int dlen){

    Fid = fopen(filename,"rb");
	if(Fid == NULL){
		printf("can`t open file\n");
		return 0;
	}

    while (!feof(Fid))//发送DATA到结束
    {
        //fread(data_buf,sizeof(char),100,Fid);//读文件读出来到缓存
        fread(data_buf,sizeof(char),dlen,Fid);//暂时读出10个
        fill_data(1,data_buf,dlen);
        //send
        sw_send(data_buf);
        memset(data_buf, 0, sizeof(data_buf));//清空缓存buf
    }
    event = EVENT_FILE_END;
    fclose(Fid);

    return 1;
}

int main (){


    // const char *file;
    // file = "recv.txt";
    // strcpy(data_buf,"Hello World !\n");
    // //printf("%lu,%d\n",sizeof("Hello World !\n"),int(strlen("Hello World !\n")));
    // for (size_t i = 0; i < 3; i++)
    // {
    //     get_data(file,data_buf,strlen(data_buf));//用srelen，不会把末尾的\0也写进去
    // }
    // return(0);

    // const char* filename;
    // filename = "snd.txt";
    // read_flie(filename,data_len);
    // printf("type is %d, seq is %d ,dlen is %d,\n send data is %s\n,check is %d\n",
    //        sndbuf.type,sndbuf.seq,sndbuf.dlen,sndbuf.data,sndbuf.check);
char* data ="hahaha";
memcpy(data,NULL,5);
return(2);

}