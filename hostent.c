/*
 * hostent.c
 *
 *  Created on: Nov 9, 2018
 *      Author: yifeifan
 */
#include	"network.h"
#if 0
//gethostbyname
int main(int argc, char **argv)
{
	char *ptr, **pptr;
	char str[INET_ADDRSTRLEN];
	struct hostent *hptr;

	while(--argc > 0)
	{
		ptr = *++argv;
		if((hptr = gethostbyname(ptr)) == NULL)
		{
			err_msg("gethostbyname error for host %s:%s",
					ptr,hstrerror(h_errno));
			continue;
		}
		printf("official hostname:%s\n",hptr->h_name);
		for(pptr = hptr->h_aliases;*pptr != NULL; pptr++)
		{
			printf("\talias:%s\n",*pptr);
		}
		switch(hptr->h_addrtype)
		{
		case AF_INET:
			pptr = hptr->h_addr_list;
			for(;*pptr != NULL; pptr++)
				printf("\taddress:%s\n",inet_ntop(hptr->h_addrtype,*pptr, str, sizeof(str)));
			break;
		default:
			err_ret("unknow address type");
			break;
		}

	}
	exit(0);

}
//gethostbyaddr

int main(int argc, char **argv)
{
	struct sockaddr_in addr;
	char *ptr,**pptr;
	struct hostent *hptr;
	while(--argc > 0)
	{
		ptr = *++argv;
		addr.sin_addr.s_addr = inet_addr(ptr);
		if((hptr = gethostbyaddr((char *)&addr.sin_addr.s_addr,4,AF_INET)) == NULL)
		{
			err_msg("gethostbyaddr error for host %s:%s",ptr,hstrerror(h_errno));
			continue;
		}
		printf("\thost name is %s",hptr->h_name);
	}
}


//gethostbyname getservbyname

int main(int argc, char **argv)
{
	int sockfd,n;
	char recvline[MAXLINE+1];
	struct sockaddr_in servaddr;
	struct in_addr **pptr;
	struct in_addr *inetaddrp[2];
	struct in_addr inetaddr;
	struct hostent *hp;
	struct servent *sp;

	if(argc != 3)
		err_quit("usage : daytimetcpcli<hostname> <service>");
	if((hp = gethostbyname(argv[1])) == NULL)
	{
		if(inet_aton(argv[1],&inetaddr) == 0)
		{
			err_quit("hostname error for %s:%s",argv[1],hstrerror(h_errno));
		}
		else
		{
			inetaddrp[0] = &inetaddr;
			inetaddrp[1] = NULL;
			pptr = inetaddrp;
		}
	}
	else
	{
		pptr = (struct in_addr **)hp->h_addr_list;
	}

	if((sp = getservbyname(argv[2],"tcp")) == NULL)
	{
		err_quit("getservbyname error for %s",argv[2]);
	}
	for(;*pptr != NULL; pptr++)
	{
		sockfd = Socket(AF_INET,SOCK_STREAM,0);
		bzero(&servaddr,sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = sp->s_port;
		memcpy(&servaddr.sin_addr,*pptr,sizeof(struct in_addr));
		printf("trying %s \n",sock_ntop((SA *)&servaddr,sizeof(servaddr)));
		if(connect(sockfd,(SA *)&servaddr,sizeof(servaddr)) == 0)
			break;
		err_ret("connect error");
		close(sockfd);
	}
	if(*pptr == NULL)
		err_quit("unable to connect");
	while((n = Read(sockfd,recvline,MAXLINE)) > 0)
	{
		recvline[n] = 0;
		fputs(recvline, stdout);
	}
	exit(0);
}
#endif
