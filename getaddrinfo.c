/*
 * getaddrinfo.c
 *
 *  Created on: Nov 13, 2018
 *      Author: yifeifan
 */


#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifndef   NI_MAXHOST
#define   NI_MAXHOST 1025
#endif
#if 0
int main(int argc, char **argv)
{
    struct addrinfo* result;
    struct addrinfo* res;
    int error;

    /* resolve the domain name into a list of addresses */
    error = getaddrinfo(argv[1], argv[2], NULL, &result);
    if (error != 0) {
        if (error == EAI_SYSTEM) {
            perror("getaddrinfo");
        } else {
            fprintf(stderr, "error in getaddrinfo: %s\n", gai_strerror(error));
        }
        exit(EXIT_FAILURE);
    }

    /* loop over all returned results and do inverse lookup */
    for (res = result; res != NULL; res = res->ai_next) {
        char hostname[NI_MAXHOST];
        error = getnameinfo(res->ai_addr, res->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0);
        if (error != 0) {
            fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(error));
            continue;
        }
        if (*hostname != '\0')
            printf("hostname: %s\n", hostname);
    }

    freeaddrinfo(result);
    return 0;
}
#endif

int tcp_conncet(const char *host, const char *serv)
{
	int  sockfd, n;
	struct addrinfo hints, *res, *ressave;
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if((n = getaddrinfo(host, serv, &hints, &res)) != 0)
		err_quit("tcp_connect error for %s, %s: %s",host, serv, gai_strerror(n));
	ressave = res;
	do{
		sockfd = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
		if(sockfd < 0)
			continue;
		if(connect(sockfd, res->ai_addr,res->ai_addrlen) == 0)
			break;
		close(sockfd);
	}while((res = res->ai_next) != NULL);
	if(res == NULL)
		err_sys("tcp_connect error for %s, %s", host, serv);
	freeaddrinfo(ressave);
	return(sockfd);

}

int
Tcp_connect(const char *host, const char *serv)
{
	return(tcp_conncet(host, serv));
}

int udp_conncet(const char *host, const char *serv)
{
	int  sockfd, n;
	struct addrinfo hints, *res, *ressave;
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if((n = getaddrinfo(host, serv, &hints, &res)) != 0)
		err_quit("tcp_connect error for %s, %s: %s",host, serv, gai_strerror(n));
	ressave = res;
	do{
		sockfd = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
		if(sockfd < 0)
			continue;
		if(connect(sockfd, res->ai_addr,res->ai_addrlen) == 0)
			break;
		close(sockfd);
	}while((res = res->ai_next) != NULL);
	if(res == NULL)
		err_sys("tcp_connect error for %s, %s", host, serv);

	freeaddrinfo(ressave);
	return(sockfd);

}

int
Udp_connect(const char *host, const char *serv)
{
	int n;
	if((n = udp_conncet(host, serv)) < 0)
	{
		err_quit("udp_connect error for %s, %s: %s",host, serv, gai_strerror(-n));
	}
	return(n);
}
