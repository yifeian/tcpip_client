/*
 * traceroute.h
 *
 *  Created on: Nov 21, 2018
 *      Author: yifeifan
 */

#ifndef TRACEROUTE_H_
#define TRACEROUTE_H_
#include "network.h"
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#if 1
#define BUFSIZE  1500

struct rec_trace{                /* format of outgoing UDP data */
	u_short rec_seq;       /*sequence number */
	u_short rec_ttl;       /*TTL packet left with */
	struct timeval rec_tv; /* time packet left */
};

/* globals   */
char recvbuf_trace[BUFSIZE];
char sendbuf_trace[BUFSIZE];

//int datalen_trace;            /* bytes of data following ICMP headers */

u_short sport, dport;
int nsent;              /*add 1 for each sento() */

pid_t pid_trace;
int probe, nprobes;
int sendfd, recvfd;
int ttl, max_ttl;
int verbose_trace;

/* function prototypes  */
const char *icmpcode_v4(int);
const char *icmpcode_v6(int);
int  recv_v4(int , struct timeval *);
int  recv_v6(int , struct timeval *);
void sig_alrm_trace(int);
void traceloop(void);
void tv_sub_trace(struct timeval *,struct timeval *);

struct proto_trace{
	const char *(*icmpcode)(int);
	int (*recv)(int , struct timeval *);
	struct sockaddr *sasend;   /*sockaddr{} for send, from getaddrinfo */
	struct sockaddr *sarecv;   /*sockaddr{} for receiving */
	struct sockaddr *salast;   /*last sockaddr{} for receving */
	struct sockaddr *sabind;   /*sockaddr{} for binding source ICMP */
	socklen_t salen;           /* length of sockadr{}s */
	int icmpporto;             /* IPPRPTO_xxx value for ICMP */
	int ttllevel;              /* setsockopt level to set ttl */
	int ttloptname;
}*pr_trace;
#ifdef IPV6
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#endif


#endif
#endif /* TRACEROUTE_H_ */
