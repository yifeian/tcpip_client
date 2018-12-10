/*
 * non_block_web.h
 *
 *  Created on: Dec 10, 2018
 *      Author: yifeifan
 */
#include "network.h"

#define MAXFILES  20
#define SERV   "80"

struct file{
	char *f_name;
	char *f_host;
	int f_fd;
	int f_flags;
}file[MAXFILES];

typedef enum _connect_flag
{
	F_CONNECTING = 1,
	F_READING    = 2,
	F_DONE       = 4,
}connect_flag;

#define  GET_CMD    "GET %s HTTP/1.0\r\n\r\n"

/* globals */
int nconn, nfiles, nlefttoconn, nlefttoread, maxfd;

fd_set rset, wset;

/* function prototypes  */

void homepage(const char *, const char *);
void start_connect(struct file *);
void write_get_cmd(struct file *);


