/*
 * ping.h
 *
 *  Created on: Nov 20, 2018
 *      Author: yifeifan
 */

#ifndef PING_H_
#define PING_H_

#include "network.h"
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define BUFSIZE  1500

char sendbuf[BUFSIZE];

int datalen;           /* bytes of data following ICMP header */
char *host;
int nsent = 1;          /* add 1 for each sendto()*/
pid_t pid;
int sockfd;
int verbose;

/* function protoytpes */

void init_v6(void);
void proc_v4(char *, ssize_t,struct msghdr *,struct timeval *);
void proc_v6(char *, ssize_t,struct msghdr *,struct timeval *);
void send_v4(void);
void send_v6(void);

void readloop(void);
void sig_alrm(int);
void tv_sub(struct timeval *,struct timeval *);
uint16_t in_cksum(uint16_t *addr, int len);

struct proto{
	void (*fproc)(char *,ssize_t, struct msghdr *,struct timeval *);
	void (*fsend)(void);
	void (*finit)(void);
	struct sockaddr *sasend;     /*sockaddr{} for send, from getaddrinfo */
	struct sockaddr *sarecv;     /* sockaddr for receving */
	socklen_t salen;             /* length of sockaddr */
	int icmpproto;
}*pr;



#include <netinet/ip6.h>
#include <netinet/icmp6.h>


#endif /* PING_H_ */
