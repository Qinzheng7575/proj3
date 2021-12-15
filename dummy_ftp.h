#ifndef __DUMMY_FTP__
#define __DUMMY_FTP__
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
//客户机
enum{STATE_CONNECT_SER, //连接服务器
     STATE_WAIT_AUTH,   //等待质询
     STATE_WAIT_RESULT, //等待认证结果
     STATE_LOGIN,       //登陆态
     STATE_WAIT_CATA,   //等待回复目录
     STATE_WAIT_GET,    //等待下载许可
     STATE_GET,         //接收数据
     STATE_PUSH,        //发送数据
     STATE_END,         //关闭
     };

enum{EVENT_BEGIN,//保留，初始态
     EVENT_CONNECT_FAIL, //连接失败
     EVENT_AUTH,         //质询报文
     EVENT_AUTH_FAIL,    //认证失败
     EVENT_AUTH_SUCCESS, //认证成功
     EVENT_ACK_D,          //ACK
     EVENT_SEARCH,       //查看目录
     EVENT_GET,          //下载命令
     EVENT_PUSH,         //上传命令
     EVENT_FILE_END,     //读到文件尾
     EVENT_DATA,         //DATA报文
     EVENT_END,          //END
     EVENT_CLOSE,        //推出命令
 

};
//服务器
enum{
     SERVER_LISTEN,
     SERVER_WAIT_ACK,
     SERVER_AUTH,
     SERVER_ING,
     SERVER_SEND_GET,
     SERVER_GET,
     SERVER_PUSH,
     SERVER_RECV,
};
enum{
     S_EVENT_BEGIN,//保留，初始态
     S_EVENT_AUTH,
     S_EVENT_REPLY,
     S_EVENT_ACK,
     S_EVENT_AUTH_SUCCESS,
     S_EVENT_END,
     S_EVENT_FILE_END,     //读到文件尾
     S_EVENT_SEARCH,
     S_EVENT_GET,
     S_EVENT_PUSH,
     S_EVENT_URL_RIGHT,
     S_EVENT_DATA,
};
#define MAXN 128
typedef struct dummy_ftp_frame{
	uint8_t   type   : 4;
	uint8_t   seq    : 4;
	uint16_t  dlen   : 8;
	uint16_t  check  : 16;
	char data[MAXN];
}DFtpFrame;

typedef struct CTRL_frame
{
    uint8_t   cmd   :  4;
    uint32_t  reserve: 28;
    char option[MAXN];
}CTRLFrame;

void mysend(size_t type, const char *data, size_t len,size_t seq = 0, size_t cmd = 0, const char *option = NULL);



#endif


