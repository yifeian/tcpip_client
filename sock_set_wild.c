/*
 * sock_set_wild.c
 *
 *  Created on: Nov 14, 2018
 *      Author: yifeifan
 */


#include "network.h"

#ifdef	IPV6
const struct in6_addr in6addr_any;
#endif

void sock_set_addr(struct sockaddr *sa, socklen_t salen, const void *addr)
{
	switch(sa->sa_family)
	{
		case AF_INET:
		{
			struct sockaddr_in *sin = (struct sockaddr_in *)sa;
			memcpy(&sin->sin_addr,addr,sizeof(struct in_addr));
			return;
		}

#ifdef	IPV6
	case AF_INET6: {
		struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

		memcpy(&sin6->sin6_addr, addr, sizeof(struct in6_addr));
		return;
	}
#endif
	}
}

void sock_set_wild(struct sockaddr *sa, socklen_t salen)
{
	const void *wildptr;
	switch(sa->sa_family)
	{
		case AF_INET:
		{
			static struct in_addr in4addr_any;
			in4addr_any.s_addr = htonl(INADDR_ANY);
			wildptr = &in4addr_any;
			break;
		}
#ifdef   IPV6
		case AF_INET6:
		{
			wildptr = &in6addr_any;
			break;
		}
#endif
		default:
			return;
	}
	sock_set_addr(sa,salen,wildptr);
}
