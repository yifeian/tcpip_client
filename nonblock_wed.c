/*
 * nonblock_wed.c
 *
 *  Created on: Dec 10, 2018
 *      Author: yifeifan
 */
#include "non_block_web.h"

void home_page(const char *host, const char *fname)
{
	int fd, n;
	char line[MAXLINE];

	fd = Tcp_connect(host, SERV);
	n = snprintf(line, sizeof(line), GET_CMD, fname);
	Write(fd, line, n);
	for(;;)
	{
		if((n = Read(fd, line, MAXLINE)) == 0)
			break;   /* server closed connection */
		printf("read %d byts of home page \n", n);
	}
	printf("end of file on home page\n");
	close(fd);
}

#if 0
int main(int argc, char **argv)
{
	int i, fd, n, maxnconn, flags, error;
	char buf[MAXLINE];
	fd_set  rs,ws;
	if(argc < 5)
		err_quit("usage : web <#conns><hostname><homepage><file1>.");
	maxnconn = atoi(argv[1]);
	nfiles = min(argc - 4,MAXFILES);
	for(i = 0; i < nfiles; i++)
	{
		file[i].f_name = argv[i+4];
		file[i].f_host = argv[2];
		file[i].f_flags = 0;
	}
	printf("nfiles = %d", nfiles);
	home_page(argv[2], argv[3]);
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	nlefttoread = nlefttoconn = nfiles;
	nconn = 0;
}
#endif

