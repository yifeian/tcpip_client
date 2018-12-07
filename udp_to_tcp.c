/*
 * udp_to_tcp.c
 *
 *  Created on: Nov 16, 2018
 *      Author: yifeifan
 */

#include "network.h"
#include <setjmp.h>
#include "unprtt.h"

#define RTT_DEBUG

static struct rtt_info rttinfo;
static int rttinit = 0;
static struct msghdr msgsend, msgrecv;  /* assumed init to 0*/
static struct hdr{
	uint32_t seq;   /*sequence */
	uint32_t ts;    /*timestamp when sent */
}sendhdr,recvhdr;

static void sig_alrm(int signo);
static sigjmp_buf jmpbuf;

/*
 * 可以通过序列号来验证这些分组是不是真的丢了，
 * 如果一个分支只是被延迟，那么将会收到两个应答，无法区分是被丢弃的分组是客户的请求还是服务器的应答
 * */

ssize_t dg_send_recv(int fd, const void *outbuff,size_t outbytes,
					 void *inbuff,size_t inbytes,const SA *destaddr,socklen_t destlen)
{
	ssize_t n;
	struct iovec iovsend[2], iovrecv[2];
	if(rttinit == 0)
	{
		rtt_init(&rttinfo);   /* first time we are called */
		rttinit = 1;
		rtt_d_flag = 1;
	}
	sendhdr.seq++;
	msgsend.msg_name = destaddr;
	msgsend.msg_namelen = destlen;
	msgsend.msg_iov = iovsend;
	msgsend.msg_iovlen = 2;
	iovsend[0].iov_base = &sendhdr;
	iovsend[0].iov_len = sizeof(struct hdr);
	iovsend[1].iov_base = outbuff;
	iovsend[1].iov_len = outbytes;

	msgrecv.msg_name = NULL;
	msgrecv.msg_namelen = 0;
	msgrecv.msg_iov = iovrecv;
	msgrecv.msg_iovlen = 2;
	iovrecv[0].iov_base = &recvhdr;
	iovrecv[0].iov_len = sizeof(struct hdr);
	iovrecv[1].iov_base = inbuff;
	iovrecv[1].iov_len = inbytes;

	Signal(SIGALRM, sig_alrm);
	rtt_newpack(&rttinfo);               /* initialize for this packet */
sendagain:
#ifdef	RTT_DEBUG
	fprintf(stderr, "send %4d: ", sendhdr.seq);
#endif
	sendhdr.ts = rtt_ts(&rttinfo);
	sendmsg(fd,&msgsend,0);
	fprintf(stderr, "wait time is %d seconds: ", rtt_start(&rttinfo));
	alarm(rtt_start(&rttinfo));      /*calc timeout value & start timer */
#ifdef	RTT_DEBUG
	rtt_debug(&rttinfo);
#endif
	if(sigsetjmp(jmpbuf, 1) != 0)
	{
		if(rtt_timeout(&rttinfo) < 0)
		{
			err_msg("dg_send_recv: no response from server, giving up");
			rttinit = 0;           /* reinit in case we're called again */
			errno = ETIMEDOUT;
			return(-1);
		}
#ifdef	RTT_DEBUG
		err_msg("dg_send_recv: timeout, retransmitting");
#endif
		goto sendagain;
	}
	do{
		n = recvmsg(fd, &msgrecv, 0);
#ifdef	RTT_DEBUG
		fprintf(stderr, "recv %4d\n", recvhdr.seq);
#endif
	}while(n < sizeof(struct hdr) || recvhdr.seq != sendhdr.seq);

	alarm(0);           /* stop sigarlm timer */
	/* calculate & store new rtt estimator values */
	rtt_stop(&rttinfo, rtt_ts(&rttinfo) - recvhdr.ts);

	return(n - sizeof(struct hdr));
}

ssize_t
Dg_send_recv(int fd, const void *outbuff, size_t outbytes,
			 void *inbuff, size_t inbytes,
			 const SA *destaddr, socklen_t destlen)
{
	ssize_t	n;

	n = dg_send_recv(fd, outbuff, outbytes, inbuff, inbytes,
					 destaddr, destlen);
	if (n < 0)
		err_quit("dg_send_recv error");

	return(n);
}


static void
sig_alrm(int signo)
{
	siglongjmp(jmpbuf, 1);
}

void dg_cli_udp(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	ssize_t n;
	char sendline[MAXLINE], recvline[MAXLINE];
	while(fgets(sendline,MAXLINE,fp) != NULL)
	{
		n = Dg_send_recv(sockfd, sendline,strlen(sendline),recvline,MAXLINE,pservaddr,servlen);
		recvline[n] = 0;
		fputs(recvline, stdout);
	}
}



