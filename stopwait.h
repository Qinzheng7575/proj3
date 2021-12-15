#ifndef __STOPWAIT_H__
#define __STOPWAIT_H__

#include <netinet/ip.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <endian.h>

// common
#define TIMEOUT 5
#define CRC_SIZE 2

typedef union sw_head {
	struct {
	#if BYTE_ORDER == BIG_ENDIAN 
		uint8_t  flag : 4;
		uint8_t  seq  : 4;
	#endif
	#if BYTE_ORDER == LITTLE_ENDIAN
		uint8_t  seq  : 4;
		uint8_t  flag : 4;
	#endif
		uint8_t dlen : 8;
	} head;
	uint16_t val;
} SWHead; //the union can be used to implement align


enum { F_SEND, F_ACK, }; //enum is local, not global!
enum { FRAME0, FRAME1, };

// sender
enum { EVENT_TIMEOUT,  EVENT_ACK, };
enum { STATE_SEND0, STATE_SEND1, STATE_WAIT0, STATE_WAIT1, };

// receiver
enum { EVENT_FRAME0, EVENT_FRAME1, };
enum { STATE_RECV0, STATE_RECV1, };
#define DELAY 5

ssize_t sw_send(const void* data, size_t len);
ssize_t sw_recv(void* buf, size_t len);
int sw_connect(uint32_t ipaddr, uint16_t port);
int sw_listen(uint32_t ipaddr, uint16_t port);
void sw_closeclt();
void sw_closesrv();



#endif

