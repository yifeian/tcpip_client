/*
 * strclithread.c
 *
 *  Created on: Dec 7, 2018
 *      Author: yifeifan
 */

#include "network.h"

static int sockfd;
static FILE *fp;
static stdineof_thread = 0;

void *copyto(void *);

void str_cli_thread(FILE *fp_arg, int sockfd_arg)
{
	char recvline[MAXLINE];
	int rc;
	pthread_t tid;
	sockfd = sockfd_arg;
	fp = fp_arg;

	if((rc = pthread_create(&tid,NULL,copyto,NULL)) != 0)
	{
		err_sys("create thread faild");
	}
	for(;;)
	{
		if((rc = Read(sockfd, recvline, MAXLINE)) == 0)
		{
			if(stdineof_thread == 1)
				return;
			else
				err_quit("str_cli :server terminated prematurely");
		}
		//printf("read data number is %d\r\n",n);
		Write(fileno(stdout), recvline, rc);
	}

}

void *copyto(void *arg)
{
	char sendline[MAXLINE];
	while(fgets(sendline, MAXLINE, fp) != NULL)
		Write(sockfd, sendline, strlen(sendline));
	stdineof_thread = 1;
	shutdown(sockfd, SHUT_WR); /* send fin */
	return(NULL);
}
