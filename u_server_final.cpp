
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h> 


#define STDIN 0//键盘输入文件描述符

struct socket_list {
	//套接字管理队列结构
	int MainSock;
	int num;
	int socket_array[256];
};
void init_list(socket_list* list)
{
	//初始化套接字
	int i = 0;
	list->num = 0;
	list->MainSock = 0;
	for (i = 0; i < 64; i++) {
		list->socket_array[i] = 0;
	}
};
void insert_list(int s, socket_list* list)
{
	//将s插入到套接字管理队列中
	int i;
	for (i = 0; i < 64; i++) {
		if (list->socket_array[i] == 0) {
			list->socket_array[i] = s;
			list->num++;
			break;
		}
	}
};
void delete_list(int s, socket_list* list)
{
	//从套接字管理队列中删除s
	int i;
	for (i = 0; i < 64; i++) {
		if (list->socket_array[i] = s) {
			list->socket_array[i] = 0;
			list->num--;
			break;
		}
	}
};
void make_fdlist(socket_list* list, fd_set* fd_list)
{
	//将管理队列中的套接字逐个添加到指定的状态队列fd_list中，
	//注意利用FD_SET()
	int i;
	//there is no mainsocket!

	for (i = 0; i < 64; i++) {

		if (list->socket_array[i] > 0) {
 //printf("i=%d is%d\n",i,list->socket_array[i]);
			FD_SET(list->socket_array[i], fd_list);
		}
	}


};

int main()
{
	int s,sock;//int instead SOCKET in Linux
	struct sockaddr_in ser_addr, remote;
	unsigned int len;
	char buf[128];

	struct socket_list sock_list;//我们建立的socket管理列表
	fd_set readfds, writefds, exceptfds;//三种socket列表，对应读、写、意外


	int i;
	unsigned long arg;
	struct timeval timeout;
	
	int retval;//承接select、recv等函数的返回值

	s = socket(AF_INET, SOCK_DGRAM, 0);//UDP
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);//环回地址for测试
	ser_addr.sin_port = htons(0x1234);

	bind(s, (struct sockaddr*)&ser_addr, sizeof(ser_addr));



	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	remote.sin_port = htons(0x4321);



	timeout.tv_sec = 2;
	timeout.tv_usec = 0;//设置等待时间
	init_list(&sock_list);
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds); 
	arg = 1;



int fd=STDIN;

//ioctlsocket(sock_list.MainSock, FIONBIO, &arg);//开启非阻塞for win
    ioctl(s,FIONBIO,&arg);//开启非阻塞for linux
ioctl(fd,FIONBIO,&arg);

	while (1) {
		//建立那三个状态队列
FD_ZERO(&readfds);

		insert_list(s, &sock_list);//it's important!!

		make_fdlist(&sock_list, &readfds);
		make_fdlist(&sock_list, &writefds);
		make_fdlist(&sock_list, &exceptfds);
		
FD_SET(fd,&readfds);
		retval = select(1024, &readfds, &writefds, &exceptfds, &timeout);
//printf("%d\n",retval);
		//从套接字上的各种事件处理
		//注意此处应设计为不断从套接字管理队列中逐个取出sock的循环模式
	



		for (i = 0; i < 64; i++) {
			if (sock_list.socket_array[i] == 0)continue;
			sock = sock_list.socket_array[i];
//printf("%d,%d\n",i,sock);
			//此时已经进入到套接字管理流程了，不需要再管新来的啥的了
			if (FD_ISSET(STDIN, &readfds)) {

				char in_buf[128];
				read(STDIN,in_buf,127);
				
				len = sizeof(remote);
				int a;
				//a=sendto(s, "ACK", 3, 0, (sockaddr*)&remote, len);
			if (strlen(in_buf)>0){

				printf("have read from keyboard:%s\n",in_buf);
				a=sendto(s, in_buf, strlen(in_buf), 0, (sockaddr*)&remote, len);
				memset(in_buf, 0x00, sizeof (char) * 128);
				printf("send ok %d\n",a);
				}
			}
			if (FD_ISSET(sock, &readfds)) {

				//len = sizeof(sock);
				len=sizeof(remote);
                                int postion=0;
				postion=recvfrom(s, buf, 127, 0, (struct sockaddr*)&remote, &len);
                                buf[postion]='\0';
				if (strlen(buf)>0){
				printf("接收到：%s\n", buf);
				memset(buf, 0x00, sizeof (char) * 128);
                                //sendto(s, "ACK", 3, 0, (sockaddr*)&remote, len);
}


				
			}
			
			
			if (FD_ISSET(sock, &writefds)) {
				//是不是该send了？？？？？

				len = sizeof(remote);
				//sendto(s, "ACK", 3, 0, (sockaddr*)&remote, len);

			}
			if (FD_ISSET(sock, &exceptfds)) {
				printf("excepted error.\n");
                	        close(sock);
			}
		}
	}
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
        close(s);
	return 0;

}


