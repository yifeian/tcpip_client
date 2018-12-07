/*
 * connect_timeo_alarm.c
 *
 *  Created on: Nov 12, 2018
 *      Author: yifeifan
 */

#include "network.h"


/*****************************alarm recvfrom and connect timeout***************************/
static void connect_alarm(int);
static void sig_alrm(int);

int connect_timeo(int sockfd, const SA *saptr, socklen_t salen, int nsec)
{
	Sigfunc *sigfunc;
	int n;
	sigfunc = Signal(SIGALRM, connect_alarm);
	if(alarm(nsec) != 0)
	{
		err_msg("connect_timeo : alarm was already set");
	}
	if((n = connect(sockfd, saptr, salen)) < 0)
	{
		close(sockfd);
		if(errno == EINTR)
			errno = ETIMEDOUT;
	}
	alarm(0);                     /* turn off the alarm */
	Signal(SIGALRM,sigfunc);      /* restore previous signal hander */
	return(n);
}

static void connect_alarm(int signo)
{
	return;                         /*just interrupt the connect() */
}

void dg_cli_timeo_alrm(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int n;
	char sendline[MAXLINE],recvline[MAXLINE+1];
	Signal(SIGALRM,sig_alrm);
	while(fgets(sendline,MAXLINE,fp) != NULL)
	{
		sendto(sockfd, sendline, strlen(sendline),0,pservaddr,servlen);
		alarm(5);
		if((n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL)) < 0)
		{
			if(errno == EINTR)
				fprintf(stderr, "socket timeout\n");
			else
				err_sys("recvfrom error");
		}
		else
		{
			alarm(0);
			recvline[n] = 0;
			fputs(recvline,stdout);
		}

	}
}

static void sig_alrm(int signo)
{
	return;                          /* just interrupt the recvfrom() */
}


/*****************************select recvfrom timeout***************************/
int readable_timeo(int fd, int sec)
{
	fd_set rset;
	struct timeval tv;

	FD_ZERO(&rset);
	FD_SET(fd, &rset);
	tv.tv_sec = sec;
	tv.tv_usec = 0;
	return(select(fd+1, &rset, NULL, NULL, &tv));
}

void dg_cli_select_timeo(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int n;
	char sendline[MAXLINE],recvline[MAXLINE+1];
	while(fgets(sendline, MAXLINE,fp) != NULL)
	{
		sendto(sockfd, sendline, strlen(sendline),0,pservaddr, servlen);
		if(readable_timeo(sockfd,5) == 0)
			fprintf(stderr, "socket timeout \n");
		else
		{
			n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
			recvline[n] = 0;
			fputs(recvline, stdout);
		}
	}
}
/*****************************SO_RCVTIMEO recvfrom timeout***************************/

void dg_cli_OPT_timeo(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int n;
	char sendline[MAXLINE],recvline[MAXLINE+1];
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv, sizeof(tv));
	while(fgets(sendline, MAXLINE, fp) != NULL)
	{
		sendto(sockfd, sendline, strlen(sendline),0,pservaddr, servlen);
		n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
		if(n < 0)
		{
			if(errno == EWOULDBLOCK)
			{
				fprintf(stderr, "socket timeout\n");
				continue;
			}
			else
				err_sys("recvfrom error");
		}
		recvline[0] = 0;
		fputs(recvline ,stdout);
	}
}


