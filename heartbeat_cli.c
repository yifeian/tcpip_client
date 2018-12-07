/*
 * heartbeat_cli.c
 *
 *  Created on: Nov 27, 2018
 *      Author: yifeifan
 */

#include "network.h"

static int servfd;
static int nsec; /* second between each alarm */
static int maxnprobes; /* probes w/no response before quit */
static int nprobes; /* probes since last server response */

static void sig_urg(int), sig_alrm(int);

void heartbeat_cli(int servfd_arg, int nsec_arg, int maxnprobes_arg)
{
	servfd = servfd_arg;
	if((nsec = nsec_arg) < 1)
		nsec = 1;
	if((maxnprobes = maxnprobes_arg) < nsec)
		maxnprobes = nsec;
	nprobes = 0;
	Signal(SIGURG, sig_urg);
	Fcntl(servfd, F_SETOWN, getpid());//接收SIGIO（异步IO信号）和SIGURG（紧急信号）的进程id
	Signal(SIGALRM, sig_alrm);
	alarm(nsec);
}

static void sig_urg(int signo)
{
	int n;
	char c;
	if((n = recv(servfd, &c, 1, MSG_OOB) < 0))
	{
		if(errno != EWOULDBLOCK)
			err_sys("recv error");
	}
	nprobes = 0;   /* reset counter */
	return;
}

static void sig_alrm(int signo)
{
	if(++nprobes > maxnprobes)
	{
		printf(stderr, "server is unreachbale \n");
		exit(0);
	}
	send(servfd,"1",1,MSG_OOB);
	alarm(nsec);
	return;
}
