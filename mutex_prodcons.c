/*
 * mutex_prodcons.c
 *
 *  Created on: Dec 10, 2018
 *      Author: yifeifan
 */

#include "unpipc.h"


struct {
  pthread_mutex_t	mutex;
  int	buff[MAXNITEMS];
  int	nput;
  int	nval;
} shared = { PTHREAD_MUTEX_INITIALIZER };

struct{
	pthread_mutex_t  mutex;
	int nput;
	int nval;
}put={
		PTHREAD_MUTEX_INITIALIZER
};

struct{
	pthread_mutex_t  mutex;
	pthread_cond_t   cond;
	int nready;
}nready={
		PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER
};
#if 0
int main(int argc, char **argv)
{
	int  i, nthreads, count[MAXNTHREADS];
	pthread_t tid_produce[MAXNTHREADS], tid_consume;
	if(argc != 3)
		err_quit("usage: prodcons2 <#items><#threads>");
	printf("iput data is %s,%s\n",argv[1],argv[2]);
	nitems = min(atoi(argv[1]), MAXNITEMS);
	nthreads = min(atoi(argv[2]),MAXNTHREADS);
	printf("nitems = %d,nthreads = %d\n ",nitems,nthreads);
	/*create produce thread */
	for(i = 0; i < nthreads; i++)
	{
		count[i] = 0;
		Pthread_create(&tid_produce[i], NULL, produce, &count[i]);
	}
	/* create consume thread */
	Pthread_create(&tid_consume, NULL, consume, NULL);
	/* wait for all producer threads ends*/
	for(i = 0; i < nthreads; i++)
	{
		Pthread_join(tid_produce[i], NULL);
		printf("count[%d] = %d\n",i,count[i]);
	}

	/* wait for all consume threads ends*/
	Pthread_join(tid_consume, NULL);
	exit(0);
}
#endif
void *produce(void *arg)
{
	int dosignal;
	for(;;)
	{
		/*
		Pthread_mutex_lock(&shared.mutex);
		if(shared.nput >= nitems)
		{
			Pthread_mutex_unlock(&shared.mutex);
			return(NULL);
		}
		shared.buff[shared.nput] = shared.nval;
		shared.nput++;
		shared.nval++;
		Pthread_mutex_unlock(&shared.mutex);
		*((int *)arg) += 1;
		*((int */
		Pthread_mutex_lock(&put.mutex);
		if(put.nput >= nitems)
		{
			Pthread_mutex_unlock(&put.mutex);
			return(NULL);   /* array is full */
		}
		buff[put.nput] = put.nval;
		put.nput++;
		put.nval++;
		Pthread_mutex_unlock(&put.mutex);

		Pthread_mutex_lock(&nready.mutex);
		dosignal = (nready.nready == 0);
		nready.nready++;
		Pthread_mutex_unlock(&nready.mutex);
		if(dosignal)
			Pthread_cond_signal(&nready.cond);
		*((int *)arg) += 1;
	}
	return(NULL);
}

void consume_wait(int i)
{
	for(;;)
	{
		Pthread_mutex_lock(&shared.mutex);
		if(i < shared.nput)
		{
			Pthread_mutex_unlock(&shared.mutex);
			return;
		}
		Pthread_mutex_unlock(&shared.mutex);
	}
}

void *consume(void *arg)
{
	int i;
	for(i = 0; i < nitems; i++)
	{
		/*
		consume_wait(i);
		if(shared.buff[i] != i)
			printf("buff[%d] = %d\n",i,shared.buff[i]);
		*/
		Pthread_mutex_lock(&nready.mutex);
		while(nready.nready == 0)
			Pthread_cond_wait(&nready.cond, &nready.mutex);
		nready.nready--;
		Pthread_mutex_unlock(&nready.mutex);
		if(buff[i] != i)
		{
			printf("buff[%d] = %d\n",i,buff[i]);
		}
	}
	return(NULL);
}

