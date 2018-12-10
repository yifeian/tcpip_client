/*
 * network.c
 *
 *  Created on: Aug 29, 2018
 *      Author: yifeifan
 */
#include "network.h"

void cli_text(FILE *fp, int sockfd)
{
	char sendline[MAXLINE],recvline[MAXLINE];
	int i = 0;
	while(Fgets(sendline, MAXLINE, fp) != NULL)
	{
		Writen(sockfd, sendline, strlen(sendline));
		if(Readline(sockfd, recvline, MAXLINE) == 0)
			err_quit("str_cli : server terminated prematurely");
		Fputs(recvline, stdout);
	}
}

void cli_binary(FILE *fp, int sockfd)
{
	char sendline[MAXLINE],recvline[MAXLINE];
	int i = 0;
	long arg[2],sum;
	while(Fgets(sendline, MAXLINE, fp) != NULL)
	{
		if(sscanf(sendline, "%ld%ld",&arg[0], &arg[1]) != 2)
		{
			printf("invalid input: %s", sendline);
			continue;
		}
		Writen(sockfd, arg, sizeof(arg));
		if(Readn(sockfd, &sum, sizeof(sum)) == 0)
			err_quit("str_cli : server terminated prematurely");
		printf("%ld\n",sum);
	}
}


void cli_select(FILE *fp, int sockfd)
{
	int maxfdp1,stdineof,n;
	fd_set rset;
	char sendline[MAXLINE],recvline[MAXLINE];
	stdineof = 0;
	FD_ZERO(&rset);
	//heartbeat_cli(sockfd, 1, 5);
	for(;;)
	{
		if (stdineof == 0)
			FD_SET(fileno(fp), &rset);
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fileno(fp),sockfd) + 1;
		if(select(maxfdp1,&rset,NULL,NULL,NULL) < 0)
		{
			if(errno == EINTR)
				continue;
			else
				err_sys("select error \n");
		}
		if(FD_ISSET(sockfd, &rset))
		{

			if((n = Read(sockfd, recvline, MAXLINE)) == 0)
			{
				if(stdineof == 1)
					return;
				else
					err_quit("str_cli :server terminated prematurely");
			}
			//printf("read data number is %d\r\n",n);
			Write(fileno(stdout), recvline, n);
		}
		if(FD_ISSET(fileno(fp), &rset))
		{

			if((n = Read(fileno(fp), sendline, MAXLINE)) == 0)
			{
				stdineof = 1;
				printf("send shundowm \r\n");
				Shutdown(sockfd, SHUT_WR);
				FD_CLR(fileno(fp),&rset);
				continue;
			}
			//printf("input data number is %d\r\n",n);
			Writen(sockfd,sendline,n);
		}
	}

}

void str_echo_stdio(int sockfd)
{
	char line[MAXLINE];
	FILE *fpin, *fpout;
	fpin = Fdopen(sockfd,"r");
	fpout = Fdopen(sockfd,"w");
	while(fgets(line,MAXLINE,fpin) != NULL)
		fputs(line,fpout);
}



#if 0

int main(int argc, char **argv)
{
	int sockfd ,n,i;
	char recvline[MAXLINE + 1];
	char sendbuff[MAXLINE];
	char buff[1024];
	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;
	int port;
	if(argc != 3)
	{
		err_quit("usage : a.out <IPadress>");
	}
/*
	if(sscanf(argv[2],"%d",&port) != 1)
	{
		err_quit("port error ");
	}
	printf("host is %s,port is %d\n",argv[1],port);

	if((sockfd = Socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		err_sys("socket error");
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	if(inet_pton(AF_INET,argv[1],&servaddr.sin_addr) <= 0)
	{
		err_quit("inet error of %s",argv[1]);
	}

	if(connect_nonb(sockfd,(SA *)&servaddr,sizeof(servaddr),2) < 0)
	{
		err_sys("connect error");
	}

	if(connect(sockfd,(SA *)&servaddr,sizeof(servaddr)) < 0)
	{
		err_sys("connect error");
	}
*/
	sockfd = tcp_udp_client(argv[1], argv[2], SOCK_STREAM);
	str_cli_thread(stdin, sockfd);

	exit(0);
}
#endif
