/*
 * unpipc.h
 *
 *  Created on: Dec 10, 2018
 *      Author: yifeifan
 */

#ifndef UNPIPC_H_
#define UNPIPC_H_

#include "network.h"



#define  MAXNITEMS   1000000
#define  MAXNTHREADS 100

int nitems;
int buff[MAXNITEMS];







void *produce(void *), *consume(void *);


#endif /* UNPIPC_H_ */
