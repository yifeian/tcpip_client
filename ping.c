/*
 * ping.c
 *
 *  Created on: Nov 20, 2018
 *      Author: yifeifan
 */


#include "network.h"
#include "ping.h"


struct proto proto_v4 = {proc_v4, send_v4,NULL,NULL,NULL,0,IPPROTO_ICMP};
#ifdef IPV6_T
struct proto proto_v6 = {proc_v6, send_v6,init_v6,NULL,NULL,0,IPPROTO_ICMPV6};
#endif
int datalen = 100;

char *sock_ntop_host(const struct sockaddr *sa,socklen_t salen)
{
	static char str[128];
	switch(sa->sa_family){
	case AF_INET:{
		struct sockaddr_in *sin = (struct sockaddr_in *)sa;
		if(inet_ntop(AF_INET,&sin->sin_addr,str,sizeof(str)) == NULL)
			return(NULL);
		return(str);
	}
	case AF_INET6:{
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;
		if(inet_ntop(AF_INET,&sin6->sin6_addr,str,sizeof(str)) == NULL)
			return(NULL);
		return(str);
	}
	default:
		snprintf(str, sizeof(str), "sock_ntop_host: unknown AF_xxx: %d, len %d",sa->sa_family, salen);
		return(str);
	}
	return(NULL);
}

struct addrinfo *Host_serv(const char *host, const char *serv, int family, int socktype)
{
	int n;
	struct addrinfo hints, *res;
	bzero(&hints,sizeof(struct addrinfo));
	hints.ai_flags = 2; /*always return canonical name */
	hints.ai_family = family;
	hints.ai_socktype = socktype;
	if((n = getaddrinfo(host,serv,&hints,&res)) != 0)
		err_quit("host_serv error for %s, %s: %s",\
				(host == NULL) ? "(no hostname)" : host,\
				(serv == NULL) ? "(no service name)" : serv,\
				gai_strerror(n));
	return(res);
}
#if 0
int main(int argc, char **argv)
{
	int c;
	struct addrinfo *ai;
	char *h;

	opterr = 0;
	while((c = getopt(argc,argv,"v")) != -1)
	{
		switch (c){
		case 'v':
			verbose++;
			break;
		case '?':
			err_quit("unrecognized option :%c",c);
			break;
		}
	}
	if(optind != argc -1)
		err_quit("usage:ping <-v> <hostname>");

	host = argv[optind];


	pid = getpid()&0xffff;    /*ICMP id field is 16 bits */
	Signal(SIGALRM, sig_alrm);
	ai = Host_serv(host,NULL,0,0);
	h = sock_ntop_host(ai->ai_addr,ai->ai_addrlen);
	printf("PING %s (%s):%d data bytes\n",ai->ai_canonname?ai->ai_canonname:h,h,datalen);

	if(ai->ai_family == AF_INET)
	{
		pr = &proto_v4;
	}
#ifdef IPV6_T
	else if(ai->ai_family == AF_INET6)
	{
		pr = &proto_v6;
	}
#endif
	else
		err_quit("unknown address family %d",ai->ai_family);
	pr->sasend = ai->ai_addr;
	pr->sarecv = Calloc(1, ai->ai_addrlen);
	pr->salen = ai->ai_addrlen;
	readloop();
	exit(0);

}
#endif
void readloop(void)
{
	int size;
	char recvbuff[BUFSIZE];
	char controlbuf[BUFSIZE];
	struct msghdr msg;
	struct iovec iov;
	ssize_t n;
	struct timeval tval;
	sockfd = Socket(pr->sasend->sa_family,SOCK_RAW,pr->icmpproto);
	setuid(getuid());
	if(pr->finit)
		(*pr->finit)();
	sig_alrm(SIGALRM);                   /* send  first packet */
	iov.iov_base = recvbuff;
	iov.iov_len = sizeof(recvbuff);
	msg.msg_name = pr->sarecv;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = controlbuf;
	for(;;)
	{
		msg.msg_namelen = pr->salen;
		msg.msg_controllen = sizeof(controlbuf);
		n = recvmsg(sockfd, &msg, 0);
		if(n < 0){
			if(errno == EINTR)
				continue;
			else
				err_sys("recvmsg error");
		}
		gettimeofday(&tval, NULL);
		(*pr->fproc)(recvbuff,n,&msg,&tval);
	}
}

void tv_sub(struct timeval *out, struct timeval *in)
{
	if((out->tv_usec -= in->tv_usec) < 0)
	{
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

void sig_alrm(int signo)
{
	(*pr->fsend)();
	alarm(1);
	return;
}

void proc_v4(char *ptr, ssize_t len,struct msghdr *msg,struct timeval *tvrecv)
{
	int hlen1, icmplen;
	double rtt;
	struct ip *ip;
	struct icmp_hdr *icmp;
	struct timeval *tvsend;
	ip = (struct ip*)ptr;          /* start of IP header */
	hlen1 = ip->ip_hl << 2;        /* length of IP header */
	if(ip->ip_p != IPPROTO_ICMP)
		return;                    /* not icmp */
	icmp = (struct icmp_hdr *)(ptr+hlen1);
	if((icmplen = len - hlen1) < 8)
		return;                      /*malformed packet */
	if(icmp->type == ICMP_ECHOREPLY)
	{
		if(icmp->icmphd_id != pid)
			return;                         /* not a response to our ECHO_REQUEST */
		if(icmplen < 16)
			return;                           /* not enouth data to use */

	tvsend = (struct timeval *)icmp->data;
	tv_sub(tvrecv,tvsend);
	rtt = tvrecv->tv_sec *1000 + tvrecv->tv_usec/1000.0;
	printf("%d bytes from %s:seq=%u,ttl=%d,rtt=%.3f ms\n",\
			icmplen,sock_ntop_host(pr->sarecv,pr->salen),icmp->icmphd_seq,ip->ip_ttl,rtt);
	}else if(verbose)
	{
		printf("  %d bytes from %s : type = %d, code = %d\n",\
				icmplen,sock_ntop_host(pr->sarecv,pr->salen),icmp->type,icmp->code);
	}

}

void send_v4(void)
{
	int len;
	struct icmp_hdr *icmp;
	icmp = (struct icmp_hdr *)sendbuf;
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->icmphd_id = pid;
	icmp->icmphd_seq = nsent++;
	memset(icmp->data,0xa5,datalen);    /* fill with pattern */
	gettimeofday((struct timeval *)icmp->data,NULL);
	len = 8 + datalen;
	icmp->checksum = 0;
	icmp->checksum = in_cksum((u_short *)icmp, len);
	Sendto(sockfd, sendbuf, len,0,pr->sasend,pr->salen);
}

uint16_t in_cksum(uint16_t *addr, int len)
{
	int nleft = len;
	uint32_t sum = 0;
	uint16_t *w = addr;
	uint16_t answer = 0;

	while(nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}

	if(nleft == 1)
	{
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;
	return(answer);

}



