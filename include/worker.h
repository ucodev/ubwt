/*
 
    ubwt - uCodev Bandwidth Tester
    Copyright (C) 2021  Pedro A. Hortas <pah@ucodev.org>

    This file is part of ubwt - uCodev Bandwidth Tester

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#ifndef UBWT_WORKER_H
#define UBWT_WORKER_H

#include "config.h"

#ifdef UBWT_CONFIG_MULTI_THREADED
#include <pthread.h>

typedef enum UBWT_WORKER_TASK_TYPES {
	UBWT_WORKER_TASK_TYPE_NVR_ANY = 1,	/* No Value Returned, (void *) type parameter */
	UBWT_WORKER_TASK_TYPE_NVR_INT		/* No Value Returned, (int) type parameter */
} ubwt_worker_task_type_t;

typedef enum UBWT_WORKER_FLAGS {
	UBWT_WORKER_FLAG_TYPE_CHILD = 1,
	UBWT_WORKER_FLAG_CANCEL_REQUESTED,
	UBWT_WORKER_FLAG_CANCEL_COMPLETED,
	UBWT_WORKER_FLAG_TASK_READY,
	UBWT_WORKER_FLAG_TASK_RUNNING,
	UBWT_WORKER_FLAG_TASK_JOINABLE
} ubwt_worker_flags_t;

typedef pthread_t ubwt_worker_t;
typedef pthread_mutex_t ubwt_worker_mutex_t;
typedef pthread_cond_t ubwt_worker_cond_t;
#ifdef UBWT_CONFIG_CUSTOM_PTHREAD_BARRIER
typedef struct ubwt_worker_barrier {
	unsigned int total, count, waiting;
	ubwt_worker_mutex_t mutex[2];
	ubwt_worker_cond_t cond[2];
} ubwt_worker_barrier_t;
#else
typedef pthread_barrier_t ubwt_worker_barrier_t;
#endif

typedef struct ubwt_worker_task {
	ubwt_worker_task_type_t type;

	union {
		void *(*f) (void *);

		void (*fi) (int);
		void (*fv) (void *);
	};

	union {
		void *v;

		int vi;
		void *vv;
	};
} ubwt_worker_task_t;

extern ubwt_worker_barrier_t __worker_barrier_global[3];
extern ubwt_worker_mutex_t   __worker_mutex_global;
extern ubwt_worker_mutex_t   __worker_mutex_cond;
extern ubwt_worker_cond_t    __worker_cond_global;
extern unsigned int          __worker_forking;

ubwt_worker_t worker_task_create(ubwt_worker_task_t *t);
void worker_task_running_set(void);
void worker_task_running_unset(void);
void worker_task_exit(void);
int worker_task_has_flag(unsigned int flag, unsigned int count);
void worker_task_join(ubwt_worker_t worker_id);
void worker_barrier_init(ubwt_worker_barrier_t *barrier, unsigned count);
int worker_barrier_wait(ubwt_worker_barrier_t *barrier);
void worker_barrier_destroy(ubwt_worker_barrier_t *barrier);
void worker_mutex_init(ubwt_worker_mutex_t *mutex);
void worker_mutex_lock(ubwt_worker_mutex_t *mutex);
void worker_mutex_trylock(ubwt_worker_mutex_t *mutex);
void worker_mutex_unlock(ubwt_worker_mutex_t *mutex);
void worker_mutex_destroy(ubwt_worker_mutex_t *mutex);
void worker_cond_init(ubwt_worker_cond_t *cond);
void worker_cond_wait(ubwt_worker_cond_t *cond, ubwt_worker_mutex_t *mutex);
void worker_cond_timedwait(ubwt_worker_cond_t *cond, ubwt_worker_mutex_t *mutex, const struct timespec *abstime);
void worker_cond_signal(ubwt_worker_cond_t *cond);
void worker_cond_broadcast(ubwt_worker_cond_t *cond);
void worker_cond_destroy(ubwt_worker_cond_t *cond);
ubwt_worker_t worker_self(void);
int worker_im_cancelled(void);
int worker_is_cancelled(ubwt_worker_t tid);
void worker_cancel(ubwt_worker_t worker_id);
#endif

void worker_init(void);
void worker_destroy(void);


#endif

