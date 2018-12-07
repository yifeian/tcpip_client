/*
 * dgclibcastl.c
 *
 *  Created on: Nov 16, 2018
 *      Author: yifeifan
 */


#include "network.h"
#include <setjmp.h>


static void recvfrom_alarm(int);


static sigjmp_buf jmpbuf;

//#define select_pipe_fix_contend
#define sigsetjmp_fix_contend
//#define maybe_contend
void dg_cli_bcast(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int n;
	const int on = 1;
	char sendline[MAXLINE], recvline[MAXLINE + 1];
	socklen_t len;
	struct sockaddr *preply_addr;
	preply_addr = malloc(servlen);
#ifdef select_pipe_fix_contend
	static int pipefd[2];
	int maxfdp1;
	fd_set rset;
	pipe(pipefd);
	maxfdp1 = max(sockfd, pipefd[0]) + 1;
#endif
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
	Signal(SIGALRM, recvfrom_alarm);
	while(fgets(sendline, MAXLINE, fp) != NULL)
	{
		sendto(sockfd,sendline, strlen(sendline), 0, pservaddr, servlen);
		alarm(3);
		for(;;)
		{
#ifdef select_pipe_fix_contend
			FD_SET(sockfd, &rset);
			FD_SET(pipefd[0],&rset);
			if((n = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0)
			{
				if(errno == EINTR)
					continue;
				else
					err_sys("select error");
			}

			if(FD_ISSET(sockfd,&rset))
			{
				len = servlen;
				n = recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr,&len);
				recvline[n] = 0;
				printf("datalen is %d,from %s : %s",n,Sock_ntop(preply_addr,len),recvline);
			}
			if(FD_ISSET(pipefd[0],&rset))
			{
				Read(pipefd[0],&n,1);
				break;
			}
#endif

#ifdef maybe_contend
			len = servlen;
			n = recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr,&len);
			if(n < 0)
			{
				printf("break recvfromerrno is %d\n ",errno);
				if(errno == EINTR)
					break;          /* waited long enough for replies */
				else
					err_sys("recvfrom error");
			}
			else
			{
				recvline[n] = 0;
				printf("from %s : %s",Sock_ntop(preply_addr,len),recvline);
			}
#endif

#ifdef sigsetjmp_fix_contend
		if(sigsetjmp(jmpbuf,1) != 0)
			break;
		len = servlen;
		n = recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr, &len);
		recvline[n] = 0;
		printf("from %s : %s",Sock_ntop(preply_addr,len),recvline);
#endif
		}
		printf("end process");
	}
	free(preply_addr);
}

static void recvfrom_alarm(int signo)
{
#ifdef select_pipe_fix_contend
	Write(pipefd[1],"",1); // write one null byte po pipe
#endif

#ifdef sigsetjmp_fix_contend
	siglongjmp(jmpbuf,1);
#endif
	return;
}
