/*
 * network.h
 *
 *  Created on: Aug 29, 2018
 *      Author: yifeifan
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netdb.h>
#include <stdbool.h>
#include <bits/types.h>
#include <errno.h>
#include <asm-generic/errno-base.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

/* Miscellaneous constants */
#define	MAXLINE		4096	/* max text line length */

#define max(a,b)    ((a) > (b) ? (a) : (b))
#define min(a,b)    ((a) > (b) ? (a) : (b))

#define SA struct sockaddr

struct icmp_hdr
{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	union
	{
		struct
		{
			uint16_t id;
			uint16_t sequence;
		}echo;
		uint32_t gateway;
		struct
		{
			uint16_t unussed;
			uint16_t mtu;
		}frag;
	}un;
	uint32_t icmp_timestap[2];
	uint8_t data[0];
#define icmphd_id un.echo.id
#define icmphd_seq un.echo.sequence
};


typedef	void	Sigfunc(int);
Sigfunc *Signal(int, Sigfunc *);

#endif /* NETWORK_H_ */
