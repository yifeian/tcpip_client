/*
 * strclinonb.c
 *
 *  Created on: Nov 12, 2018
 *      Author: yifeifan
 */

#include "network.h"

char *gf_time(void)
{
	struct timeval tv;
	static char str[30];
	char *ptr;
	if(gettimeofday(&tv, NULL) < 0)
	{
		err_sys("gettimeofday error");
	}
	ptr = ctime(&tv.tv_sec);
	strcpy(str,&ptr[11]);

	snprintf(str+8,sizeof(str)-8,".%06ld",tv.tv_usec);
	return (str);
}
/*non block type with one process */
void str_cli_nonblock(FILE *fp, int sockfd)
{
	int maxfdp1, val, stdineof;
	ssize_t n, nwritten;
	fd_set rset, wset;
	char to[MAXLINE], fr[MAXLINE];
	char  *toiptr, *tooptr,*friptr,*froptr;
	/* SET SOCKFD NON BLOCK MODE */
	val = Fcntl(sockfd, F_GETFL, 0);
	Fcntl(sockfd, F_SETFL, val | O_NONBLOCK);
	/* SET STDOUT_FILENO NON BLOCK MODE */
	val = Fcntl(STDOUT_FILENO, F_GETFL, 0);
	Fcntl(sockfd, F_SETFL, val | O_NONBLOCK);
	/* SET STDIN_FILENO NON BLOCK MODE */
	val = Fcntl(STDIN_FILENO, F_GETFL, 0);
	Fcntl(sockfd, F_SETFL, val | O_NONBLOCK);
	/* initialize buffer pointers */
	toiptr = tooptr = to;
	friptr = froptr = fr;

	stdineof = 0;
	maxfdp1 = max(max(STDOUT_FILENO,STDIN_FILENO),sockfd) + 1;
	for(; ;)
	{
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		if(stdineof == 0 && toiptr < &to[MAXLINE])
			FD_SET(STDIN_FILENO, &rset);
		if(friptr < &fr[MAXLINE])
			FD_SET(sockfd, &rset);
		if(tooptr != toiptr)
			FD_SET(sockfd, &wset);
		if(froptr != friptr)
			FD_SET(STDOUT_FILENO, &wset);
		Select(maxfdp1,&rset, &wset, NULL, NULL);

		if(FD_ISSET(STDIN_FILENO, &rset))
		{
			if((n = read(STDIN_FILENO,toiptr,&to[MAXLINE] - toiptr)) < 0)
			{
				if(errno != EWOULDBLOCK)
					err_sys("read error on stdin");
			}
			else if(n == 0)
			{
				fprintf(stderr,"%s:EOF on stdin\n",gf_time());
				stdineof = 1;        /*all done with stdin */
				if(tooptr == toiptr)
					shutdown(sockfd, SHUT_WR);      /* SEND FIN */
			}
			else
			{
				fprintf(stderr, "%s : read %d bytes from stdin\n",gf_time(),n);
				toiptr += n;             /* JUST READ */
				FD_SET(sockfd, &wset);	 /*Try and wirte to socket below */
			}
		}
		if(FD_ISSET(sockfd,&rset))
		{
			if((n = read(sockfd, friptr, &fr[MAXLINE] - friptr)) < 0)
			{
				if(errno != EWOULDBLOCK)
					err_sys("read error on socket");
			}else if(n == 0)
			{
				fprintf(stderr,"%s : EOF on socket \n",gf_time());
				if(stdineof)
					return;         /* normal terminated */
			}else
			{
				fprintf(stderr, "%s : read %d bytes from socket\n",gf_time(),n);
				friptr += n;
				FD_SET(STDOUT_FILENO, &wset);   /* try and write below */
			}
		}
		if(FD_ISSET(STDOUT_FILENO, &wset) && ((n = friptr - froptr) > 0))
		{
			if((nwritten = write(STDOUT_FILENO,froptr,n)) < 0)
			{
				if(errno != EWOULDBLOCK)
					err_sys("write error to stdout");
			}
			else
			{
				fprintf(stderr,"%s:wrote %d bytes to stdout\n",gf_time(),nwritten);
				froptr += nwritten;
				if(froptr == friptr)
					froptr = friptr = fr;   /* back to beginning of buffer */
			}
		}
		if(FD_ISSET(sockfd, &wset) && (n = toiptr - tooptr) > 0)
		{
			if((nwritten = write(sockfd, tooptr,n)) < 0)
			{
				if(errno != EWOULDBLOCK)
					err_sys("write error to socket");
			}
			else
			{
				fprintf(stderr,"%s:wrote %d bytes to socket \n",gf_time(),nwritten);
				tooptr += nwritten;
				if(tooptr ==  toiptr)
				{
					tooptr = toiptr = to;
					if(stdineof)
					{
						shutdown(sockfd, SHUT_WR);
					}
				}
			}
		}

	}

}
/*   strlic fork  */
void str_cli_fork(FILE *fp, int sockfd)
{
	pid_t pid;
	char sendline[MAXLINE],recvline[MAXLINE];
	if((pid = fork()) == 0)
	{
		/* child : server -> stdout */
		while(Readline(sockfd,recvline,MAXLINE) > 0)
			fputs(recvline,stdout);
		kill(getppid(),SIGTERM);           /* in case parent still running */
	}
	/* parents : stdin -> server */
	while(fgets(sendline, MAXLINE, fp) != NULL)
		Writen(sockfd, sendline, strlen(sendline));
	shutdown(sockfd, SHUT_WR);               /* EOF on stdin, send FIN */
	pause();
	return;
}

/* *** connect non block***************************/
int connect_nonb(int sockfd, const SA *saptr, socklen_t salen, int nsec)
{
	int flags, n, error;
	socklen_t len;
	fd_set rset, wset;
	struct timeval tval;

	flags = Fcntl(sockfd, F_GETFL, 0);
	Fcntl(sockfd, F_SETFL, flags|O_NONBLOCK);
	error = 0;
	if((n = connect(sockfd, saptr, salen)) < 0)
	{
		if((errno != EINPROGRESS))           /* connect return non block */
			return(-1);
	}
	if(n == 0)
	{
		printf("connect already success \n");
		goto done;                         /* connect completed immediately */
	}
	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	tval.tv_sec = nsec;
	tval.tv_usec = 0;
	/*
	 * select和非阻塞connect的两条原则：
		1. 当连接成功时，描述符变为可写.
		2. 当连接建立遇到错误时，描述符变得既可读又可写.
	*/
	if((n = select(sockfd+1, &rset, &wset,NULL,(nsec?&tval:NULL))) == 0)
	{
		close(sockfd);          /***********timeout**********/
		errno = ETIMEDOUT;
		return(-1);
	}
	if(FD_ISSET(sockfd,&rset) || FD_ISSET(sockfd, &wset))
	{
		len = sizeof(error);
		/* if connect success return 0 */
		if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR,&error,&len) < 0)
			return(-1);
	}
	else
		err_quit("select error : sockfd not set");
done:
	Fcntl(sockfd, F_SETFL,flags);    /* restore file status flags */
	if(error)                        /* connect failed  */
	{
		close(sockfd);
		errno = error;
		return(-1);
	}
	return(0);
}
