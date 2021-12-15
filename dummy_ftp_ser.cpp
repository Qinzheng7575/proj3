#include "dummy_ftp.h"
#include "stopwait.h"
#include "CHAP.h"
/*
总流程：

*/
#define EACH_DATA_LEN 100
static DFtpFrame sndbuf;
static CTRLFrame ctrlbuf;
static DFtpFrame *rebuf; //接受报文转结构体
static CTRLFrame *rectrlbuf;
char data_buf[MAXN];    //读取文件的buf
char recv_buf[MAXN];    //接收到报文存放的地方
char data_option[MAXN]; //专门用来把DATA报文的data字段和CTRL的option字段存在这里
char file_name[50]; //客户机发来的请求文件名
const char* my_file_name;     //上传下载文件名
char send_buf[MAXN];

FILE *Fid;
static int cltsock;
int event;
int *state;
// 设置服务器地址和端口号
uint32_t srvaddr = INADDR_ANY;
uint16_t my_port = 0x1234;
int temp;

static inline int check()
{
    // TODO
    return 1;
}

// fill_data使用如：fill_data(1,data_buf,dlen);
// type标明发送帧类型(1是数据报，0是控制报)

static inline void fill_data(size_t type, const char *data, size_t len,
                             size_t seq = 0, size_t cmd = 0, const char *option = NULL){
    len = len > MAXN ? MAXN : len;
    sndbuf.type = type;
    sndbuf.seq = seq;
    sndbuf.dlen = len;

    if (type == 0){
        ctrlbuf.cmd = cmd;
        if(option == NULL){memset(ctrlbuf.option,0,sizeof(ctrlbuf.option));}
        else{
        memcpy(ctrlbuf.option, option, len - 32); //填CTRL报文,那些命令
        }

        memcpy(sndbuf.data, &ctrlbuf, len);
    }
    else{
        memcpy(sndbuf.data, data, len);
    }
    sndbuf.check = check();
}

// read_flie函数输入参数为要发送的文件名，和每次发送的长度:read_flie(filename,data_len);
// read_file干的事情：打开文件，分批次读取到data_buf，读取一批则调用fill_data填充一批到sndbuf
//接着sw_send一批
static inline int read_flie(const char *filename, int dlen)
{

    Fid = fopen(filename, "rb");
    if (Fid == NULL)
    {
        printf("can`t open file\n");
        return 0;
    }

    while (!feof(Fid)) //发送DATA到结束
    {
        int read_num = fread(data_buf, sizeof(char), dlen, Fid); //读文件读出来到缓存，暂时读出10个
        fill_data(1, data_buf, read_num);                        //把从文件读取的东西装在数据报文中,之所以用read_num是因为可能读不了dlen长
        // fill_data填到sndbuf结构体里了，但最后发出去的时候还是个字符串
        memcpy(send_buf, &sndbuf, sizeof(sndbuf));
        sw_send(send_buf, sizeof(send_buf));
        memset(data_buf, 0, sizeof(data_buf)); //清空缓存buf
    }
    event = S_EVENT_FILE_END;
    fclose(Fid);

    return 1;
}

// get_data在handel_data之后出现，目的是取得正确的file_name
//使用：get_data(file,data_buf,strlen(data_buf));
//用strlen，不会把末尾的\0也写进去，可以多次调用
static int get_data(const char *filename, const char *recv_buf, int len)
{
    Fid = fopen(filename, "a+"); //因为是分块接收，使用a+参数不擦除原有内容
    if (Fid == NULL){ //打开失败
        return (0);
    }
    fwrite(recv_buf, len, sizeof(char), Fid);

    fclose(Fid);
    return (1);
}

// 调用sw_send API发送数据
void mysend(size_t type, const char *data, size_t len,
                 size_t seq , size_t cmd , const char *option ){
    fill_data(type, data, len, seq, cmd, option);

    memcpy(send_buf, &sndbuf, sizeof(sndbuf));
    sw_send(send_buf, sizeof(send_buf));
}
static void myrecv(char* buf, size_t len){
    sw_recv(buf,len);
    memset(buf,0,sizeof(buf));
}

// handle_data处理数据,data_option负责传出data或者option字段（文件名/目录）的内容
static int s_handle_data(int event,char* recv_buf,char* data_option){
    
    rebuf = (DFtpFrame*)recv_buf;
    uint8_t type = (*rebuf).type;

    if(type == 0){  //报文类型为CTRL
        recv_buf = recv_buf + 4;
        rectrlbuf = (CTRLFrame*)recv_buf;
        switch (rectrlbuf->cmd)
        {
        case 0:     //cmd字段为0代表push报文，触发事件S_EVENT_PUSH
            event = S_EVENT_PUSH;
            break;
        case 1:     //cmd字段为1代表get报文，触发事件S_EVENT_GET
            event = S_EVENT_GET;
            break;
        case 2:
            event = S_EVENT_SEARCH;
            break;
        case 4:
            event = S_EVENT_END;
            break;
        }
        
        data_option = rectrlbuf->option;
    }
    else if(type == 1){     //报文类型为MSG
        event = S_EVENT_DATA;
        data_option = rebuf->data;
    }
    else if(type == 2){     //报文类型为AUTH
        //AUTH报文利用帧格式的seq字段标识质询阶段
        //定义如下：0-request; 1-质询; 2-回应; 3-认证成功; 4-认证失败

        switch (rebuf->seq)
        {
        case 0:     //seq字段为0代表收到request报文，触发事件S_EVENT_AUTH
            event = S_EVENT_AUTH;
            break;
        case 2:
            event = S_EVENT_REPLY;
        }
        data_option = rebuf->data; 
    }
    return 0;
}

static int myconnect()
{ //启动
    // sw_connect()调用API
    if (sw_connect(srvaddr, my_port) < 0){
        event = EVENT_CONNECT_FAIL;
        return -1;
    }
    return 0;
}

static void mylisten(){
    sw_listen(srvaddr,my_port);
} 

static void send_dummy(const char *data) { printf("%s\n", data); }


static inline int main_loop_server(int *state, size_t len)
{
    printf("server begin\n");
    mylisten();
    //等待对面连接
    while (1)
    {
        switch (*state)
        {
        case SERVER_LISTEN: //监听
            myrecv(recv_buf, MAXN);
            s_handle_data(event, recv_buf, data_option);
            if (event == S_EVENT_AUTH)
            { //这里的event不一定，得和客户端配合着来
                send_dummy("质询报文\n");
                chap_auth(2,NULL,0,1);
                *state = SERVER_WAIT_ACK;
            }
            break;

        case SERVER_WAIT_ACK: //等待回应
            myrecv(recv_buf, MAXN);
            s_handle_data(event, recv_buf, data_option);
            if (event == S_EVENT_REPLY)
            { //同上
                *state = SERVER_AUTH;
            }
            break;

        case SERVER_AUTH: //验证身份
            myrecv(recv_buf, MAXN);
            s_handle_data(event, recv_buf, data_option);
            temp = chap_result(data_option,2,NULL,0,0,0,NULL);//已经发送认证结果报文

            event = temp ? S_EVENT_AUTH_SUCCESS : S_EVENT_END;
            if (event == S_EVENT_AUTH_SUCCESS){
                *state = SERVER_ING;
            }
            else if (event == S_EVENT_END){
                *state = SERVER_LISTEN;
            }
            break;

        case SERVER_ING: //服务态
            myrecv(recv_buf, MAXN);
            s_handle_data(event, recv_buf, data_option);
            switch (event)
            {
            case S_EVENT_END:
                send_dummy("要关闭了\n");
                *state = SERVER_ING;
                break;

            case S_EVENT_SEARCH:
                send_dummy("ACK(选项：url)");
                mysend(0,my_file_name,strlen(my_file_name),0,3);
                *state = SERVER_ING;
                break;
            case S_EVENT_GET:
                *state = SERVER_SEND_GET;
                break;
            case S_EVENT_PUSH:
                memcpy(file_name,data_option,strlen(data_option));//长度可能有问题？
                *state = SERVER_RECV;
                break;
            default:
                break;
            }
            break;

        case SERVER_SEND_GET: // GET请求判断
            //判断路径是否合法
            
            event = strcmp(my_file_name,file_name) ? S_EVENT_END : S_EVENT_URL_RIGHT;
            if(event == EVENT_END){
                mysend(0,NULL,0,0,4);
                *state = SERVER_LISTEN;
            }
            else if (event == S_EVENT_URL_RIGHT)
                send_dummy("ACK（选项：许可）\n");
                mysend(0,NULL,0,0,3);
                *state = SERVER_PUSH;
            break;

        case SERVER_RECV: //接受数据
            myrecv(recv_buf, MAXN);
            s_handle_data(event, recv_buf, data_option);
            if (event == S_EVENT_END){
                *state = SERVER_ING;
            }
            else if (event == S_EVENT_DATA){
                get_data(file_name, recv_buf, len); 
            }
            break;

        case SERVER_PUSH: //发送数据
            read_flie(file_name, EACH_DATA_LEN);
            if (event == S_EVENT_FILE_END)
            {
                send_dummy("END报文");
                mysend(0,NULL,0,0,4);
                *state = SERVER_ING;
            }
            break;


        default:
            break;
        }
    }
}

int main()
{
    printf("it's time to test !\n");
    printf("%d\n",*state);
    *state = SERVER_LISTEN;
    event = S_EVENT_BEGIN;
    printf("1\n");
    my_file_name = "test.txt";
    
    main_loop_server(state, MAXN);
    return (0);
}