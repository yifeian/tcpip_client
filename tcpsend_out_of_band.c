/*
 * tcpsend_out_of_band.c
 *
 *  Created on: Nov 19, 2018
 *      Author: yifeifan
 */

#include "network.h"
#if 0
int main(int argc, char **argv)
{
	int sockfd;
	if(argc < 3)
		err_quit("usage:<host>,<port>");
	sockfd = Tcp_connect(argv[1],argv[2]);
	Write(sockfd, "123",3);
	printf("wrote 3 bytes of normal data\n");
	//sleep(1);
	Send(sockfd, "4",1,MSG_OOB);
	printf("wrote 1 byte of OBB data\n");
	//sleep(1);
	Write(sockfd, "5",1);
	printf("wrote 2 bytes of normal data\n");

	//sleep(1);
	Send(sockfd,"6",1,MSG_OOB);
	printf("wrote 1 byte of OBB data\n");
	//sleep(1);
	Write(sockfd, "7",1);
	printf("wrote 2 bytes of normal data\n");
	//sleep(1);

	exit(0);
}



/*
 * 1. 即便因为流量控制而停止发送数据了，tcp仍然发送外带数据的通知
   2. 在外带数据到达前，接收进程可呢个被通知说发送进程以后发送了外带数据，
   （SIGURG或者select），如果接收进场接着指定MSG_OOB调用recv，外带数据却尚未到达
    recv将返回EWOULDBLOCK错误。
 * */
int main(int argc, char **argv)
{
	int sockfd, size;
	char buff[16384];
	if(argc != 3)
		err_quit("usage: <host> <prot>");
	sockfd = Tcp_connect(argv[1],argv[2]);
	size = 32768;
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
	Write(sockfd, buff, 16384);
	printf("worte 16384 bytes of normal data\n");
	sleep(5);
	Send(sockfd,"a",1,MSG_OOB);
	printf("worte q byte of OOB data\n");
	Write(sockfd, buff, 1024);
	printf("worte 1024 bytes of normal data\n");
	exit(0);
}
#endif
