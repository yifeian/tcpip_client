/*
 * Wrappthread.c
 *
 *  Created on: Dec 10, 2018
 *      Author: yifeifan
 */

#include "unpipc.h"

void Pthread_create(pthread_t *tid, const pthread_attr_t *attr,\
		void *(*func)(void *), void *arg)
{
	int n;
	if((n = pthread_create(tid, attr, func, arg)) == 0)
		return;
	errno = n;
	err_sys("pthread create error");
}
void Pthread_join(pthread_t tid, void **status)
{
	int n;
	if((n = pthread_join(tid, status)) == 0)
		return;
	errno = n;
	err_sys("pthread_hoin error");
}

void Pthread_mutex_lock(pthread_mutex_t *mptr)
{
	int  n;
	if((n = pthread_mutex_lock(mptr)) == 0)
		return;
	errno = n;

	err_sys("pthread_mutex_lock error");
}

void Pthread_mutex_unlock(pthread_mutex_t *mptr)
{
	int n;
	if((n = pthread_mutex_unlock(mptr)) == 0)
		return;
	errno = n;
	printf("errno is %d\n",errno);
	err_sys("pthread_mutex_unlock error");
}

void Pthread_cond_signal(pthread_cond_t *cptr)
{
	int		n;

	if ( (n = pthread_cond_signal(cptr)) == 0)
		return;
	errno = n;
	err_sys("pthread_cond_signal error");
}

void
Pthread_cond_wait(pthread_cond_t *cptr, pthread_mutex_t *mptr)
{
	int		n;

	if ( (n = pthread_cond_wait(cptr, mptr)) == 0)
		return;
	errno = n;
	err_sys("pthread_cond_wait error");
}

sem_t *Sem_open(const char *pathname, int oflag,...)
{
	sem_t *sem;
	va_list ap;
	mode_t mode;
	unsigned int value;

	if(oflag & O_CREAT)
	{
		va_start(ap, oflag);
		mode = va_arg(ap, mode_t);
		value = va_arg(ap,unsigned int);
		if((sem = sem_open(pathname, oflag, mode, value)) == SEM_FAILED)
			err_sys("sem_open error for %s",pathname);
		va_end(ap);
	}
	else
	{
		if((sem = sem_open(pathname, oflag)) == SEM_FAILED)
			err_sys("sem_openerror for %s",pathname);
	}
	return(sem);
}
void
Sem_close(sem_t *sem)
{
	if (sem_close(sem) == -1)
		err_sys("sem_close error");
}

void
Sem_unlink(const char *pathname)
{
	if (sem_unlink(pathname) == -1)
		err_sys("sem_unlink error");
}


char *
px_ipc_name(const char *name)
{
	char	*dir, *dst, *slash;

	if ( (dst = malloc(PATH_MAX)) == NULL)
		return(NULL);

		/* 4can override default directory with environment variable */
	if ( (dir = getenv("PX_IPC_NAME")) == NULL) {
#ifdef	POSIX_IPC_PREFIX
		dir = POSIX_IPC_PREFIX;		/* from "config.h" */
#else
		dir = "/tmp/";				/* default */
#endif
	}
		/* 4dir must end in a slash */
	slash = (dir[strlen(dir) - 1] == '/') ? "" : "/";
	snprintf(dst, PATH_MAX, "%s%s%s", dir, slash, name);

	return(dst);			/* caller can free() this pointer */
}
/* end px_ipc_name */

char *
Px_ipc_name(const char *name)
{
	char	*ptr;

	if ( (ptr = px_ipc_name(name)) == NULL)
		err_sys("px_ipc_name error for %s", name);
	printf("Px_ipc_name is %s\n",ptr);
	return(ptr);
}
void
Sem_wait(sem_t *sem)
{
	if (sem_wait(sem) == -1)
		err_sys("sem_wait error");
}

int
Sem_trywait(sem_t *sem)
{
	int		rc;

	if ( (rc = sem_trywait(sem)) == -1 && errno != EAGAIN)
		err_sys("sem_trywait error");
	return(rc);
}

/* include Sem_post */
void
Sem_post(sem_t *sem)
{
	if (sem_post(sem) == -1)
		err_sys("sem_post error");
}

void
Sem_init(sem_t *sem, int pshared, unsigned int value)
{
	if (sem_init(sem, pshared, value) == -1)
		err_sys("sem_init error");
}

void
Sem_destroy(sem_t *sem)
{
	if (sem_destroy(sem) == -1)
		err_sys("sem_destroy error");
}


