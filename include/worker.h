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
	UBWT_WORKER_TASK_TYPE_INT = 1,
	UBWT_WORKER_TASK_TYPE_VOID
} ubwt_worker_task_type_t;

typedef pthread_t ubwt_worker_t;
typedef pthread_barrier_t ubwt_worker_barrier_t;
typedef pthread_mutex_t ubwt_worker_mutex_t;

typedef struct ubwt_worker_task {
	ubwt_worker_task_type_t type;

	union {
		void (*fi) (int);
		void (*fv) (void *);
	};

	union {
		int vi;
		void *vv;
	};
} ubwt_worker_task_t;

extern ubwt_worker_barrier_t __worker_barrier_global[2];
extern ubwt_worker_mutex_t   __worker_mutex_global;

void worker_create(ubwt_worker_task_t *t);
void worker_exit(void);
void worker_wait(ubwt_worker_barrier_t *barrier);
void worker_lock(ubwt_worker_mutex_t *mutex);
void worker_trylock(ubwt_worker_mutex_t *mutex);
void worker_unlock(ubwt_worker_mutex_t *mutex);
ubwt_worker_t worker_self(void);
int worker_im_cancelled(void);
int worker_is_joinable(ubwt_worker_t tid);
void worker_cancel(ubwt_worker_t worker_id);
void worker_join(ubwt_worker_t worker_id);
#endif

void worker_init(void);
void worker_destroy(void);


#endif

