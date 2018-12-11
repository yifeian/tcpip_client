/*
 * POSIX_SEM_prodcons.c
 *
 *  Created on: Dec 11, 2018
 *      Author: yifeifan
 */

#include "unpipc.h"

#define NBUFF 10
#define SEM_MUTEX   "mutex1"
#define SEM_NEMPTY  "nempty1"
#define SEM_NSTORED "nstored1"

struct {
	int buff[NBUFF];
	int nput;
	int nputval;
	sem_t mutex, nempty, nstored;
}posix_shared;

void *produce_sem(void* ),*consume_sem(void *);

int main(int argc, char **argv)
{
	int i,count[MAXNTHREADS];
	pthread_t tid_produce[MAXNTHREADS], tid_consume;

	if(argc != 3)
		err_quit("usage : prodcons 1 <items> <#producers>");
	nitems = atoi(argv[1]);
	nproducers = min(atoi(argv[2]), MAXNTHREADS);

	/* create three semaphores
	posix_shared.mutex = Sem_open(Px_ipc_name(SEM_MUTEX),O_EXCL,\
			FILE_MODE, 1);
	posix_shared.nempty = Sem_open(Px_ipc_name(SEM_NEMPTY),O_EXCL,\
			FILE_MODE, NBUFF);
	posix_shared.nstored = Sem_open(Px_ipc_name(SEM_NSTORED),O_EXCL,\
			FILE_MODE, 0);
	*/
	Sem_init(&posix_shared.mutex, 0, 1);
	Sem_init(&posix_shared.nempty, 0, NBUFF);
	Sem_init(&posix_shared.nstored, 0, 0);
	/* create one producer and one consumer thread */
	for(i = 0; i < nproducers; i++)
	{
		count[i] = 0;
		Pthread_create(&tid_produce[i], NULL,produce_sem,&count[i]);
	}

	Pthread_create(&tid_consume,NULL,consume_sem,NULL);
	for(i = 0; i < nproducers; i++)
	{
		Pthread_join(tid_produce[i], NULL);
		printf("count[%d] = %d\n",i,count[i]);
	}

	Pthread_join(tid_consume, NULL);

	/* remove the semapores */
	Sem_destroy(&posix_shared.mutex);
	Sem_destroy(&posix_shared.nempty);
	Sem_destroy(&posix_shared.nstored);
	exit(0);
}

void *produce_sem(void *arg)
{
	int i;
	for(;;)
	{
		printf("thread %d is working\n",pthread_self());
		Sem_wait(&posix_shared.nempty);
		Sem_wait(&posix_shared.mutex);
		if(posix_shared.nput >= nitems)
		{
			Sem_post(&posix_shared.nempty);
			Sem_post(&posix_shared.mutex);
			return(NULL);
		}
		posix_shared.buff[posix_shared.nput%NBUFF] = posix_shared.nputval;
		printf("write in data is %d, i is %d\n",posix_shared.buff[i%NBUFF],posix_shared.nput);
		posix_shared.nput++;
		posix_shared.nputval++;

		Sem_post(&posix_shared.mutex);
		Sem_post(&posix_shared.nstored);
		*((int *)arg) += 1;
	}
	return(NULL);
}

void *consume_sem(void *arg)
{
	int i;
	for(i = 0; i < nitems; i++)
	{
		printf("thread %d is working\n",pthread_self());
		Sem_wait(&posix_shared.nstored);
		Sem_wait(&posix_shared.mutex);
		if(posix_shared.buff[i%NBUFF] != i)
			printf("buff[%d] = %d\n",i,posix_shared.buff[i%NBUFF]);
		else
			printf("correct buff[%d] = %d\n",i,posix_shared.buff[i%NBUFF]);
		Sem_post(&posix_shared.mutex);
		Sem_post(&posix_shared.nempty);
	}
	return(NULL);
}

