// 服务器连接.cpp : 建立连接，收到客户机发送来的字符串后，回送一个ACK字符串给客户机。
//
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
int main(int argc, char* argv[])
{
	int s,newsock;
	sockaddr_in ser_addr;
	sockaddr_in remote_addr;
	int len;
	char buf[128];



	s = socket(AF_INET,SOCK_STREAM,0);
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(0x1234);
	
	bind(s,(sockaddr*)&ser_addr,sizeof(ser_addr));

	listen(s,0);
	while(1){
		len = sizeof(remote_addr);
		newsock = accept(s,(sockaddr*)&remote_addr,(socklen_t*)&len);
		recv(newsock,buf,sizeof(buf),0);
		send(newsock,"ACK",3,0);
		close(newsock);
	}
	close(s);
	return 0;
}

