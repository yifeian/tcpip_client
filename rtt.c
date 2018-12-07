/*
 * rtt.c
 *
 *  Created on: Nov 19, 2018
 *      Author: yifeifan
 */

#include "unprtt.h"

/*
 * calculate the RTO value based on current estimators:
 * smoothed RTT plus for times the deviation
 */
int		rtt_d_flag = 0;		/* debug flag; can be set by caller */


#define RTT_TROCALC(prt)  ((ptr)->rtt_srtt+(4.0*(ptr)->rtt_rttvar))

static float
rtt_minmax(float rto)
{
	if(rto < RTT_RXTMIN)
		rto = RTT_RXTMIN;
	else if(rto > RTT_RXTMAX)
		rto = RTT_RXTMAX;
	return(rto);
}

void rtt_init(struct rtt_info *ptr)
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	ptr->rtt_base = tv.tv_sec;   /* sec since 1/1/1970 at start */
	ptr->rtt_rtt = 0;
	ptr->rtt_srtt = 0;
	ptr->rtt_rttvar = 0.75;
	ptr->rtt_rto = rtt_minmax(RTT_TROCALC(ptr));
	/*first rto at (srtt+(4*rttval)) = 3 seconds */
}

uint32_t rtt_ts(struct rtt_info *ptr)
{
	uint32_t ts;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	ts = ((tv.tv_sec - ptr->rtt_base) * 1000) + (tv.tv_usec/1000);
	return (ts);
}

void rtt_newpack(struct rtt_info *ptr)
{
	ptr->rtt_nrexmt = 0;
}

int rtt_start(struct rtt_info *ptr)
{
	return ((int)(ptr->rtt_rto + 0.5));  /* round float to to int*/
	/* return value can be used as : alarm(rtt_start(&foo)) */
}

void rtt_stop(struct rtt_info *ptr, uint32_t ms)
{
	double delta;
	ptr->rtt_rtt = ms/1000.0;    /* measured RTT in secondes */
	/*
	 * update our estimators of RTT and mean deviation of RTT
	 * page 477
	 */
	delta = ptr->rtt_rtt - ptr->rtt_srtt;
	ptr->rtt_srtt += delta/8;    /* g = 1/8  */
	if(delta < 0.0)
		delta = -delta;
	ptr->rtt_rttvar += (delta - ptr->rtt_rttvar)/4;   /* h = 1/4 */
	ptr->rtt_rto = rtt_minmax(RTT_TROCALC(ptr));
}

int rtt_timeout(struct rtt_info *ptr)
{
	ptr->rtt_rto *= 2;   /* next TRO */
	if(++ptr->rtt_nrexmt > RTT_MAXNREXMT)
		return(-1);                      /* time to give up for this packet */
	return(0);
}

/*
 * Print debugging information on stderr, if the "rtt_d_flag" is nonzero.
 */

void
rtt_debug(struct rtt_info *ptr)
{
	if (rtt_d_flag == 0)
		return;

	fprintf(stderr, "rtt = %.3f, srtt = %.3f, rttvar = %.3f, rto = %.3f\n",
			ptr->rtt_rtt, ptr->rtt_srtt, ptr->rtt_rttvar, ptr->rtt_rto);
	fflush(stderr);
}

