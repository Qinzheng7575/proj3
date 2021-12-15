// 连接客户.cpp : 与服务器建立连接后，向服务器发送一个字符串，收到服务器应答后，关闭连接。
//
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
int main(int argc, char* argv[])
{
	int s;
	sockaddr_in local,server;

	char *buf = "I am a client";
	char recvbuf[128];



	s = socket(AF_INET,SOCK_STREAM,0);
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(0x3412);
	
	bind(s,(sockaddr*)&local,sizeof(local));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	server.sin_port = htons(0x1234);
	
	connect(s,(sockaddr*)&server,sizeof(server));
	
	send(s,buf,strlen(buf),0);
	recv(s,recvbuf,sizeof(recvbuf),0);
	close(s);

	return 0;
}

