#include "stopwait.h"
#include "dummy_ftp.h"

#include "CHAP.h"
/*
总流程：
*/
static DFtpFrame sndbuf;
static CTRLFrame ctrlbuf;
static DFtpFrame *rebuf; //接受报文转结构体
static CTRLFrame *rectrlbuf;
char data_buf[MAXN];    //读取文件的buf
char recv_buf[MAXN];    //接收到报文存放的地方
char data_option[MAXN]; //专门用来把DATA报文的data字段和CTRL的option字段存在这里
char file_name[50];     //上传下载文件名
char send_buf[MAXN];
FILE *Fid;
static int cltsock;
int event;
int *state;
// 设置服务器地址和端口号
uint32_t srvaddr = INADDR_LOOPBACK;
uint16_t port = 0x1234;


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
    if (Fid == NULL){
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
    
    event = EVENT_FILE_END;
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
// static void mysend(size_t type, const char *data, size_t len,size_t seq = 0, size_t cmd = 0, const char *option = NULL){
void mysend(size_t type, const char *data, size_t len,size_t seq , size_t cmd , const char *option ){

    fill_data(type, data, len, seq, cmd, option);

    memcpy(send_buf, &sndbuf, sizeof(sndbuf));
    sw_send(send_buf, sizeof(send_buf));
}
static void myrecv(char* buf, size_t len){
    sw_recv(buf,len);
    memset(buf,0,sizeof(buf));
}

// handle_data处理数据,data_option负责传出data或者option字段（文件名/目录）的内容
static int handle_data(int event,char* recv_buf,char* data_option){
    
    rebuf = (DFtpFrame*)recv_buf;
    uint8_t type = (*rebuf).type;

    if(type == 0){  //报文类型为CTRL
        recv_buf = recv_buf + 4;
        rectrlbuf = (CTRLFrame*)recv_buf;
        switch (rectrlbuf->cmd)
        {
        case 3:     //cmd字段为3代表ack报文，触发事件EVENT_ACK_D
            event = EVENT_ACK_D;
            break;
        case 4:     //cmd字段为4代表end报文，触发事件EVENT_END
            event = EVENT_END;
            break;
        }
        
        data_option = rectrlbuf->option;
    }
    else if(type == 1){     //报文类型为MSG
        event = EVENT_DATA;
        data_option = rebuf->data;
    }
    else if(type == 2){     //报文类型为AUTH
        //AUTH报文利用帧格式的seq字段标识质询阶段
        //定义如下：0-request; 1-质询; 2-回应; 3-认证成功; 4-认证失败

        switch (rebuf->seq)
        {
        case 1:     //seq字段为1代表收到质询报文，触发事件EVENT_AUTH
            event = EVENT_AUTH;
            break;
        case 3:     //seq字段为3代表认证成功报文，触发事件EVENT_AUTH_SUCCESS
            event = EVENT_AUTH_SUCCESS;
            break;
        case 4:     //seq字段为4代表认证失败报文，触发事件EVENT_AUTH_FAIL
            event = EVENT_AUTH_FAIL;
            break;
        }
        data_option = rebuf->data; 
    }
    return 0;
}

static int myconnect()
{ //启动
    // sw_connect()调用API
    if (sw_connect(srvaddr, port) < 0){
        event = EVENT_CONNECT_FAIL;
        return -1;
    }
    return 0;
}

static void send_dummy(const char *data) { printf("%s\n", data); }

static inline int main_loop_client(int *state, size_t len)
{

    int data_type;
    int ctrl_type;
    myconnect();
    while (1)
    {
        //接受数据这块，完全不需要select接收，因为API已经
        //提供了稳定的接收器了，调用就直接收到报文

        /*识别状态，对于下面的状态，现在event已经是ACK了，
        要不要进一步判断报文内容自己决定使用handle_data()
        */
        switch (*state)
        {
        case STATE_CONNECT_SER:
            if (event == EVENT_CONNECT_FAIL){break;}
            else if(event == EVENT_BEGIN){
                send_dummy("Request报文\n");
                chap_req(2,NULL,0,0);//此时为ATH报文，且seq=0代表是request
                *state = STATE_WAIT_AUTH;
            }

            break;

        case STATE_WAIT_AUTH:
            myrecv(recv_buf, MAXN);                   //稳定收一个报文，停等直到你给我为止
            handle_data(event, recv_buf, data_option); //判断报文内容，在此函数内已经改变了event
            if (event == EVENT_AUTH){
                send_dummy("回应报文");
                //调用CHAP发送回复
                chap_reply(data_option, 2, NULL, strlen(data_option), 2);
                *state = STATE_WAIT_RESULT;
            }
            break;

        case STATE_WAIT_RESULT:
            myrecv(recv_buf, MAXN);
            handle_data(event, recv_buf, data_option);
            
            *state = (event == EVENT_AUTH_FAIL) ? STATE_END : STATE_LOGIN;
            break;

        case STATE_LOGIN:
            printf("请输入你的指令\n查看目录：search\n,下载：get\n上传：push\n退出：close\n");
            char *order;
            scanf("%s", order);
            if (order == "search"){event = EVENT_SEARCH;}
            else if (order == "get"){event = EVENT_GET;}
            else if (order == "push"){event = EVENT_PUSH;}
            else if (order == "close"){event = EVENT_END;}
            //--------------------------
            switch (event)
            {
            case EVENT_SEARCH:
                send_dummy("search报文");
                mysend(0,NULL,0,0,2);
                *state = STATE_WAIT_CATA;
                break;
            case EVENT_GET:
                send_dummy("Get报文");
                mysend(0,NULL,strlen(file_name),0,1,file_name);
                *state = STATE_WAIT_GET;
                break;
            case EVENT_PUSH:
                send_dummy("Push报文");
                mysend(0,NULL,strlen(file_name),0,0,file_name);
                *state = STATE_PUSH;
                break;
            case EVENT_CLOSE:
                //退出
                send_dummy("close报文");
                mysend(0,NULL,0,0,5);
                *state = STATE_END;
                break;
            default:
                break;
            }
            break;

        case STATE_WAIT_CATA:
            myrecv(recv_buf, MAXN);
            handle_data(event, recv_buf, data_option);
            memcpy(file_name,data_option,strlen(data_option));//长度可能有问题？
            //这里需要handledata把filename存下来
            if (event == EVENT_ACK_D){
                //打印目录
                printf("%s",file_name);
                *state = STATE_LOGIN;
            }
            break;

        case STATE_WAIT_GET://等待下载许可
            myrecv(recv_buf, MAXN);
            handle_data(event, recv_buf, data_option);
            if (event == EVENT_ACK_D){
                *state = STATE_GET;
            }
            else if (event == EVENT_END){
                *state = STATE_END;
            }
            break;

        case STATE_PUSH:
            read_flie(file_name, len);
            send_dummy("大文件"); //这里发完时会把event更改
            if (event == EVENT_FILE_END){
                *state = STATE_LOGIN;
                send_dummy("END报文");
                mysend(0,NULL,0,0,4);
            }
            break;

        case STATE_GET:
            //接受报文，处理数据
            myrecv(recv_buf, MAXN);
            handle_data(event, recv_buf, data_option);
            get_data(file_name, data_option, strlen(data_option));
            if (event == EVENT_END){
                *state = STATE_LOGIN;
            }
            break;

        case STATE_END:
            // close
            break;

        default:
            break;
        }
    }
}

int main()
{
    printf("it's time to test !\n");
    *state = STATE_CONNECT_SER;
    event  =EVENT_BEGIN;
    main_loop_client(state, MAXN);
    return (0);
}