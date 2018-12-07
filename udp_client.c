/*
 * udp_client.c
 *
 *  Created on: Oct 26, 2018
 *      Author: yifeifan
 */


#include "network.h"


#define NDG 180000             /* datagrams to send */
#define DGLEN 1400            /*length of each datagram*/
void dg_cli_count(FILE *fp, int sockfd,const SA *pservaddr, socklen_t servlen)
{
	socklen_t len;
	struct sockaddr_in cliaddr;
	connect(sockfd, pservaddr, servlen);
	len = sizeof(cliaddr);
	getsockname(sockfd, (SA *)&cliaddr,&len);
	printf("local address %s\n",Sock_ntop((SA *)&cliaddr, len));

}



void dg_cli(FILE *fp, int sockfd,const SA *pservaddr, socklen_t servlen)
{
	int n;
	char sendline[MAXLINE], recvline[MAXLINE+1];
	socklen_t len;
	struct sockaddr *preply_addr;
	preply_addr = Malloc(servlen);
	while(Fgets(sendline, MAXLINE, fp) != NULL)
	{
		Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
		len = servlen;
		n = Recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr,&len);
		if(len != servlen || memcmp(pservaddr,preply_addr,len) != 0)
		{
			recvline[n] = 0;
			Fputs(recvline, stdout);
			printf("reply from %s (ignored)\n",Sock_ntop(preply_addr,len));
			continue;
		}
		printf("recv from %s \n",Sock_ntop(preply_addr,len));
		recvline[n] = 0;
		Fputs(recvline, stdout);
	}
}
static void recvfrom_alarm_broadcast(int);
static int pipefd[2];
void dg_cli_select(FILE *fp, int sockfd,const SA *pservaddr, socklen_t servlen)
{
	int maxfdp1,stdineof,n;
	fd_set rset;
	const int on = 1;
	socklen_t len;
	struct sockaddr *preply_addr;
	char sendline[MAXLINE],recvline[MAXLINE];
	pipe(pipefd);
	FD_ZERO(&rset);
	preply_addr = Malloc(servlen);
	setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));
	maxfdp1 = max(max(fileno(fp),sockfd),pipefd[0]) + 1;
	Signal(SIGALRM, recvfrom_alarm_broadcast);
	for(;;)
	{
		FD_SET(fileno(fp), &rset);
		FD_SET(sockfd, &rset);
		FD_SET(pipefd[0],&rset);


		if(Select(maxfdp1,&rset,NULL,NULL,NULL) < 0)
		{
			if(errno == EINTR)
				continue;
			else
				err_sys("select error");
		}
		if(FD_ISSET(sockfd, &rset))
		{
			//printf("sock is selected \r\n");
			n = Recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr,&len);
			if(len != servlen || memcmp(pservaddr,preply_addr,len) != 0)
			{
				recvline[n] = 0;
				Fputs(recvline, stdout);
				printf("reply from %s (ignored)\n",Sock_ntop(preply_addr,len));
				continue;
			}
			printf("recv from %s \n",Sock_ntop(preply_addr,len));
			recvline[n] = 0;
			Fputs(recvline, stdout);
		}
		if(FD_ISSET(fileno(fp), &rset))
		{
			//printf("file is selected\r\n");
			if((n = Read(fileno(fp), sendline, MAXLINE)) == 0)
			{
				FD_CLR(fileno(fp),&rset);
				continue;
			}
			//printf("read data number is %d\r\n",n);
			Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
			len = servlen;
		}
		if(FD_ISSET(pipefd[0], &rset))
		{
			printf("timeout \n");
			Read(pipefd[0],&n,1); /* time expired */
			break;
		}
	}
	free(preply_addr);
}

static void recvfrom_alarm_broadcast(int signo)
{
	Write(pipefd[1],"",1);
	return;
}

void dg_cli_connect(FILE *fp, int sockfd,const SA *pservaddr, socklen_t servlen)
{
	int n;
	char sendline[MAXLINE], recvline[MAXLINE+1];
	socklen_t len;
	struct sockaddr *preply_addr;
	preply_addr = Malloc(servlen);
	connect(sockfd, (SA *)pservaddr, servlen);
	while(Fgets(sendline, MAXLINE, fp) != NULL)
	{
		Write(sockfd, sendline, strlen(sendline));
		len = servlen;
		n = Read(sockfd, recvline, MAXLINE);

		recvline[n] = 0;
		Fputs(recvline, stdout);
	}
}



#if 0
int main(int argc, char **argv)
{
	int sockfd;
	uint16_t port;
	struct sockaddr_in servaddr;

	if(argc != 3)
		err_quit("usage : udPcli <PORT>");
	if(sscanf(argv[2],"%d",&port) != 1)
	{
		err_quit("port error ");
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	//dg_cli_select(stdin, sockfd, (SA *)&servaddr, sizeof(servaddr));
	dg_cli_select(stdin, sockfd, (SA *)&servaddr, sizeof(servaddr));
	exit(0);
}
#endif
