/*
 * traceroute.c
 *
 *  Created on: Nov 21, 2018
 *      Author: yifeifan
 */

#include "traceroute.h"



struct proto_trace proto_v4_trace = {icmpcode_v4, recv_v4, NULL, NULL, NULL, NULL,0,\
						 IPPROTO_ICMP, IPPROTO_IP,IP_TTL};

#ifdef IPV6
struct proto_trace proto_v4 = {icmpcode_v6, recv_v6, NULL, NULL, NULL, NULL,0,\
						 IPPROTO_ICMPV6, IPPROTO_IPV6,IPV6_UNICAST_HOPS};
#endif

int datalen_trace = sizeof(struct rec_trace);   /* defaults  */
int max_ttl = 30;
int nprobes = 3;;
u_short dport = 32768 + 666;

void sock_set_port(struct sockaddr *sa, socklen_t salen, int port)
{
	switch(sa->sa_family){
	case AF_INET:{
		struct sockaddr_in *sin = (SA *)sa;
		sin->sin_port = port;
		return;
	}
#ifdef IPV6
	case AF_INET6:{
		struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;
		sin6->sin6_port = port;
		return;
	}
#endif
	}

}
#if 0
int main(int argc, char **argv)
{
	int c;
	struct addrinfo *ai;
	char *h;
/*
	opterr = 0;
	while((c = getopt(argc,argv,"m:v")) != -1)
	{
		switch (c){
		case 'm':
			if((max_ttl = atoi(optarg)) <= 1)
				err_quit("invalid -m value");
			break;
		case 'v':
			verbose_trace++;
			break;
		case '?':
			err_quit("unrecognized option :%c",c);
			break;
		}
	}
	if(optind != argc -1)
		err_quit("usage:ping <-v> <hostname>");
	host_trace = argv[optind];
*/

	char *host_trace = "www.baidu.com";
	pid_trace = getpid()&0xffff;    /*ICMP id field is 16 bits */
	Signal(SIGALRM, sig_alrm_trace);
	ai = Host_serv(host_trace,NULL,0,0);
	h = sock_ntop_host(ai->ai_addr,ai->ai_addrlen);
	printf("traceroute to  %s (%s):%d hops max,%d data bytes\n",\
			ai->ai_canonname?ai->ai_canonname:h,h,max_ttl,datalen_trace);
	/* initialize according to protocol */
	if(ai->ai_family == AF_INET)
	{
		pr_trace = &proto_v4_trace;
	}
	else
		err_quit("unknown address family %d", ai->ai_family);
	pr_trace->sasend = ai->ai_addr;
	pr_trace->sarecv = Calloc(1, ai->ai_addrlen);
	pr_trace->salast = Calloc(1, ai->ai_addrlen);
	pr_trace->sabind = Calloc(1, ai->ai_addrlen);
	pr_trace->salen = ai->ai_addrlen;
	traceloop();
	exit(0);
}
#endif
int sock_cmp_addr(const struct sockaddr *sa1, const struct sockaddr *sa2, socklen_t salen)
{
	if(sa1->sa_family != sa2->sa_family)
		return(-1);
	switch(sa1->sa_family){
	case AF_INET:{
		return(memcmp( &((struct sockaddr_in *)sa1)->sin_addr,
				       &((struct sockaddr_in *)sa2)->sin_addr,
				       sizeof(struct in_addr)));
	}

#ifdef	IPV6
	case AF_INET6: {
		return(memcmp( &((struct sockaddr_in6 *) sa1)->sin6_addr,
					   &((struct sockaddr_in6 *) sa2)->sin6_addr,
					   sizeof(struct in6_addr)));
	}
#endif

#ifdef	AF_UNIX
	case AF_UNIX: {
		return(strcmp( ((struct sockaddr_un *) sa1)->sun_path,
					   ((struct sockaddr_un *) sa2)->sun_path));
	}
#endif

#ifdef	HAVE_SOCKADDR_DL_STRUCT
	case AF_LINK: {
		return(-1);		/* no idea what to compare here ? */
	}
#endif

	}
	return (-1);
}

void traceloop(void)
{
	int seq, code, done;
	double rtt_t;
	struct rec_trace *prec;
	struct timeval tvrecv;

	recvfd = Socket(pr_trace->sasend->sa_family,SOCK_RAW, pr_trace->icmpporto);
	setuid(getuid());
	sendfd = Socket(pr_trace->sasend->sa_family,SOCK_DGRAM, 0);
	pr_trace->sabind->sa_family = pr_trace->sasend->sa_family;
	sport = (getpid() & 0xffff) | 0x8000; /* our source UDP port */
	sock_set_port(pr_trace->sabind,pr_trace->salen,htons(sport));
	Bind(sendfd, pr_trace->sabind,pr_trace->salen);
	sig_alrm_trace(SIGALRM);
	seq = 0;
	done = 0;
	for(ttl = 1; ttl < max_ttl && done == 0; ttl++)
	{
		Setsockopt(sendfd, pr_trace->ttllevel, pr_trace->ttloptname,&ttl,sizeof(int));
		bzero(pr_trace->salast,pr_trace->salen);
		printf("%2d",ttl);
		fflush(stdout);

		for(probe = 0; probe < nprobes; probe++)
		{
			prec = (struct rec_trace*)sendbuf_trace;
			prec->rec_seq = ++seq;
			prec->rec_ttl = ttl;
			gettimeofday(&prec->rec_tv,NULL);
			sock_set_port(pr_trace->sasend,pr_trace->salen,htons(dport+seq));
			Sendto(sendfd, sendbuf_trace, datalen_trace, 0, pr_trace->sasend,pr_trace->salen);
			if((code = (*pr_trace->recv)(seq, &tvrecv)) == -3)
				printf(" *");    /* timeout, no reply */
			else{
				char str[NI_MAXHOST];
				if(sock_cmp_addr(pr_trace->sarecv,pr_trace->salast, pr_trace->salen) != 0)
				{
					if(getnameinfo(pr_trace->sarecv,pr_trace->salen,str,sizeof(str),NULL,0,0) == 0)
					{
						printf(" %s (%s)", str, sock_ntop_host(pr_trace->sarecv,pr_trace->salen));
					}
					else
						printf(" %s", sock_ntop_host(pr_trace->sarecv,pr_trace->salen));
					memcpy(pr_trace->salast,pr_trace->sarecv,pr_trace->salen);
				}
				tv_sub(&tvrecv,&prec->rec_tv);
				rtt_t = tvrecv.tv_sec * 1000.0 + tvrecv.tv_usec/1000.0;
				printf("  %.3f ms", rtt_t);
				if(code == -1)    /*port unreachable ; at destination */
					done++;
				else if(code >= 0)
					printf(" (ICMP %s)", (*pr_trace->icmpcode)(code));
			}
			fflush(stdout);
		}
		printf("\n");
	}

}

int gotalarm;
/*
 * Return  : -3 on timeout
 *           -2 ON ICMP time exceeded in transit (caller keeps going)
 *           -1 on ICMP unreachable (caller is done)
 *           >=0 return value is some other ICMP unreachable code
 * */

int recv_v4(int seq, struct timeval *tv)
{
	int hlen1, hlen2, icmplen, ret;
	socklen_t len;
	ssize_t n;
	struct ip *ip, *hip;
	struct icmp_hdr *icmp;
	struct udphdr *udp;
	gotalarm = 0;
	alarm(3);
	for(; ;)
	{
		if(gotalarm)
			return(-3);
		len = pr_trace->salen;
		n = recvfrom(recvfd, recvbuf_trace,sizeof(recvbuf_trace),0,pr_trace->sarecv,&len);
		if(n < 0)
		{
			if(errno == EINTR)
				continue;
			else
				err_sys("recvfrom error");
		}
		ip = (struct ip *)recvbuf_trace;
		hlen1 = ip->ip_hl << 2;
		icmp = (struct icmp_hdr *)(recvbuf_trace+hlen1);
		if((icmplen = n - hlen1) < 8)
			continue;          /* not enouth data to look at imcp header */
		if(icmp->type == ICMP_TIMXCEED && icmp->code == ICMP_TIMXCEED_INTRANS)
		{
			if(icmplen < 8 + sizeof(struct ip))
				continue;               /* not enouth data to look at inner IP */
			hip = (struct ip *)(recvbuf_trace + hlen1 + 8);
			hlen2 = hip->ip_hl << 2;
			if(icmplen < hlen2 + 8 + 4)
				continue;               /* not enough data to look at UDP ports */
			udp = (struct udphdr *)(recvbuf_trace + hlen1 + 8 + hlen2);
			if(hip->ip_p == IPPROTO_UDP &&\
			   udp->source == htons(sport)&&\
			   udp->dest == htons(dport+seq))
			{
				ret = -2;       /* we hit an intermediate router */
				break;
			}
		}
		else if(icmp->type == ICMP_UNREACH)
		{
			if(icmplen < 8 + sizeof(struct ip))
				continue;
			hip = (struct ip *)(recvbuf_trace + hlen1 +8);
			hlen2 = hip->ip_hl << 2;
			if(icmplen < 8+ hlen2 + 4)
				continue;
			udp = (struct udp *)(recvbuf_trace+ hlen1 + 8 + hlen2);
			if(hip->ip_p == IPPROTO_UDP &&\
			   udp->source == htons(sport)&&\
			   udp->dest == htons(dport+seq))
			{
				if(icmp->code == ICMP_UNREACH_PORT)
					ret = -1;         /* have reached destination */
				else
					ret = icmp->code;
				break;
			}
		}
		if(verbose_trace)
			printf(" (from %s : tpye = %d, code = %d)\n",\
					sock_ntop_host(pr_trace->sarecv,pr_trace->salen),\
					icmp->type,icmp->code);
	}
	alarm(0);
	gettimeofday(tv, NULL);
	return (ret);
}

void sig_alrm_trace(int signo)
{
	gotalarm = 1;           /* set flag to note that alarm occurred */
	return;					/* and interrupt the recvfrom () */
}

const char *
icmpcode_v4(int code)
{
	static char errbuf[100];
	switch (code) {
	case  0:	return("network unreachable");
	case  1:	return("host unreachable");
	case  2:	return("protocol unreachable");
	case  3:	return("port unreachable");
	case  4:	return("fragmentation required but DF bit set");
	case  5:	return("source route failed");
	case  6:	return("destination network unknown");
	case  7:	return("destination host unknown");
	case  8:	return("source host isolated (obsolete)");
	case  9:	return("destination network administratively prohibited");
	case 10:	return("destination host administratively prohibited");
	case 11:	return("network unreachable for TOS");
	case 12:	return("host unreachable for TOS");
	case 13:	return("communication administratively prohibited by filtering");
	case 14:	return("host recedence violation");
	case 15:	return("precedence cutoff in effect");
	default:	sprintf(errbuf, "[unknown code %d]", code);
				return errbuf;
	}
}

