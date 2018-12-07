/*
 * unprtt.h
 *
 *  Created on: Nov 16, 2018
 *      Author: yifeifan
 */

#ifndef UNPRTT_H_
#define UNPRTT_H_

#include "network.h"

struct rtt_info{
	float rtt_rtt;   /* most recent measured RTT, in secondes */
	float rtt_srtt;  /* smoothed RTT estimator, in secondes */
	float rtt_rttvar; /* smoothed mean deviation , in secondes */
	float rtt_rto; /* current RTO to use, in secondes */
	int rtt_nrexmt; /* time retransmitted : 0, 1, 2 ...*/
	uint32_t rtt_base;  /* sec since 1/1/1970 at start */
};

#define RTT_RXTMIN   1 /*	min retransmit timeout value , in seconds */
#define RTT_RXTMAX   60 /* max retransmit timeout value, in seconds */
#define RTT_MAXNREXMT  3 /* max times to retransmit */

/* function prototypes  */

void rtt_debug(struct rtt_info *);
void rtt_init(struct rtt_info *);
void rtt_newpack(struct rtt_info *);
int rtt_start(struct rtt_info *);
void rtt_stop(struct rtt_info *,uint32_t );

int rtt_timeout(struct rtt_info *);
uint32_t rtt_ts(struct rtt_info *);
extern int rtt_d_flag;


#endif /* UNPRTT_H_ */
