/*
 * tcp_udp_client.c
 *
 *  Created on: Dec 7, 2018
 *      Author: yifeifan
 */

#include "network.h"

int tcp_udp_client(const char *host, const char *serv, int socktype)
{
	int sockfd, n;
	struct addrinfo hints, *res, *ressave;
	bzero(&hints, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = socktype;
	if((n = getaddrinfo(host, serv, &hints, &res)) != 0)
		err_quit("udp_client error for %s, %s: %s",\
				host, serv, gai_strerror(n));
	ressave = res;
	do
	{
		if(res->ai_family == AF_INET)
		{
			sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
			if(sockfd >= 0)
			{
				if(socktype == SOCK_DGRAM)
					break;
				else if(socktype == SOCK_STREAM)
				{
					if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
						break;
					close(sockfd);
				}
			}
			else
				continue;
		}

	}while((res = res->ai_next) != NULL);
	if(res == NULL)
		err_sys("error occuer for %s, %s", host, serv);
	freeaddrinfo(ressave);
	return(sockfd);
}
