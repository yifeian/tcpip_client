/*
 * unpipc.h
 *
 *  Created on: Dec 10, 2018
 *      Author: yifeifan
 */

#ifndef UNPIPC_H_
#define UNPIPC_H_

#include "network.h"

/* Miscellaneous constants */
#ifndef	PATH_MAX			/* should be in <limits.h> */
#define	PATH_MAX	1024	/* max # of characters in a pathname */
#endif

#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define	POSIX_IPC_PREFIX "/"

#define  MAXNITEMS   1000000
#define  MAXNTHREADS 100

int nitems, nproducers;
int buff[MAXNITEMS];







void *produce(void *), *consume(void *);


#endif /* UNPIPC_H_ */
