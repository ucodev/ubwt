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

#include "config.h"

#ifdef UBWT_CONFIG_MULTI_THREADED

#include <stdlib.h>
#include <pthread.h>

#include "bitops.h"
#include "current.h"
#include "error.h"
#include "worker.h"

ubwt_worker_barrier_t __worker_barrier_global[2]; /* 0: straight, 1: reverse */
ubwt_worker_mutex_t   __worker_mutex_global; /* used by 'current' handlers */
ubwt_worker_mutex_t   __worker_mutex_cond;
ubwt_worker_cond_t    __worker_cond_global;

static void *_worker_task_init(void *arg) {
	ubwt_worker_task_t *t = arg;

	current_fork(t);

	worker_mutex_unlock(current->worker_mutex_cond);

	switch (t->type) {
		case UBWT_WORKER_TASK_TYPE_NVR_ANY: {
			t->fv(t->vv);
		} break;

		case UBWT_WORKER_TASK_TYPE_NVR_INT: {
			t->fi(t->vi);
		} break;

		default: error_abort(__FILE__, __LINE__, "_worker_task_init");
	}

	return NULL;
}

void worker_task_running_set(void) {
	current_running_set();
}

void worker_task_running_unset(void) {
	current_running_unset();
}

void worker_task_exit(void) {
	int retval = 0;

	current_exit();

	pthread_exit((void *) &retval);
}

int worker_task_has_flag(unsigned int flag) {
	return current_children_has_flag(flag);
}

void worker_task_join(ubwt_worker_t tid) {
	current_join(tid);

	if (pthread_join(tid, NULL)) error_abort(__FILE__, __LINE__, "pthread_join");
}

ubwt_worker_t worker_task_create(ubwt_worker_task_t *t) {
	ubwt_worker_t tid;

	worker_mutex_lock(current->worker_mutex_cond); /* Will unlock on _worker_task_init thread after current_fork() */

	if (pthread_create(&tid, NULL, _worker_task_init, t)) {
		worker_mutex_unlock(current->worker_mutex_cond);

		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_WORKER_CREATE_FAILED, "worker_task_create(): pthread_create()");
		error_no_return();
	}

	return tid;
}

void worker_barrier_init(ubwt_worker_barrier_t *barrier, unsigned count) {
	if (pthread_barrier_init(barrier, NULL, count)) error_abort(__FILE__, __LINE__, "pthread_barrier_init");
}

void worker_barrier_wait(ubwt_worker_barrier_t *barrier) {
	pthread_barrier_wait(barrier);
}

void worker_barrier_destroy(ubwt_worker_barrier_t *barrier) {
	pthread_barrier_destroy(barrier);
}

void worker_mutex_init(ubwt_worker_mutex_t *mutex) {
	if (pthread_mutex_init(mutex, NULL)) error_abort(__FILE__, __LINE__, "pthread_mutex_init");
}

void worker_mutex_lock(ubwt_worker_mutex_t *mutex) {
	if (pthread_mutex_lock(mutex)) error_abort(__FILE__, __LINE__, "pthread_mutex_lock");
}

void worker_mutex_trylock(ubwt_worker_mutex_t *mutex) {
	if (pthread_mutex_trylock(mutex)) error_abort(__FILE__, __LINE__, "pthread_mutex_trylock");
}

void worker_mutex_unlock(ubwt_worker_mutex_t *mutex) {
	if (pthread_mutex_unlock(mutex)) error_abort(__FILE__, __LINE__, "pthread_mutex_unlock");
}

void worker_mutex_destroy(ubwt_worker_mutex_t *mutex) {
	pthread_mutex_destroy(mutex);
}

void worker_cond_init(ubwt_worker_cond_t *cond) {
	if (pthread_cond_init(cond, NULL)) error_abort(__FILE__, __LINE__, "pthread_cond_init");
}

void worker_cond_wait(ubwt_worker_cond_t *cond, ubwt_worker_mutex_t *mutex) {
	if (pthread_cond_wait(cond, mutex)) error_abort(__FILE__, __LINE__, "pthread_cond_wait");
}

void worker_cond_timedwait(ubwt_worker_cond_t *cond, ubwt_worker_mutex_t *mutex, const struct timespec *abstime) {
	pthread_cond_timedwait(cond, mutex, abstime);
}

void worker_cond_signal(ubwt_worker_cond_t *cond) {
	if (pthread_cond_signal(cond)) error_abort(__FILE__, __LINE__, "pthread_cond_signal");
}

void worker_cond_broadcast(ubwt_worker_cond_t *cond) {
	if (pthread_cond_signal(cond)) error_abort(__FILE__, __LINE__, "pthread_cond_broadcast");
}

void worker_cond_destroy(ubwt_worker_cond_t *cond) {
	pthread_cond_destroy(cond);
}

ubwt_worker_t worker_self(void) {
	return pthread_self();
}

int worker_im_cancelled(void) {
	int ret = 0;

	worker_mutex_lock(&current->worker_mutex_local);

	ret = bit_test(&current->worker_flags, UBWT_WORKER_FLAG_CANCEL_REQUESTED);

	worker_mutex_unlock(&current->worker_mutex_local);

	return ret;
}

int worker_is_cancelled(ubwt_worker_t tid) {
	int ret = 0;
	struct ubwt_current *c = current_get(tid);

	worker_mutex_lock(&c->worker_mutex_local);

	ret = bit_test(&c->worker_flags, UBWT_WORKER_FLAG_CANCEL_COMPLETED);

	worker_mutex_unlock(&c->worker_mutex_local);

	return ret;
}

void worker_cancel(ubwt_worker_t tid) {
	struct ubwt_current *c = current_get(tid);

	worker_mutex_lock(&c->worker_mutex_local);

	bit_set(&c->worker_flags, UBWT_WORKER_FLAG_CANCEL_REQUESTED);

	worker_mutex_unlock(&c->worker_mutex_local);

	if (pthread_cancel(tid)) error_abort(__FILE__, __LINE__, "pthread_cancel");
}

void worker_init(void) {
	if (current->config->worker_straight_first_count && current->config->bidirectional) {
		worker_barrier_init(&__worker_barrier_global[0], current->config->worker_straight_first_count);
	} else if (current->config->worker_straight_first_count) {
		worker_barrier_init(&__worker_barrier_global[0], current->config->worker_straight_first_count);
	} else if (current->config->bidirectional) {
		worker_barrier_init(&__worker_barrier_global[0], current->config->worker_count);
	} else {
		error_abort(__FILE__, __LINE__, "worker_barrier_init");
	}

	if (current->config->worker_reverse_first_count && current->config->bidirectional) {
		worker_barrier_init(&__worker_barrier_global[1], current->config->worker_reverse_first_count);
	} else if (current->config->worker_reverse_first_count) {
		worker_barrier_init(&__worker_barrier_global[1], current->config->worker_reverse_first_count);
	} else if (current->config->bidirectional) {
		worker_barrier_init(&__worker_barrier_global[1], current->config->worker_count);
	}

	worker_mutex_init(&__worker_mutex_global);
	worker_mutex_init(&__worker_mutex_cond);
	worker_cond_init(&__worker_cond_global);
}

void worker_destroy(void) {
	if (current->config->worker_straight_first_count || current->config->bidirectional)
		worker_barrier_destroy(&__worker_barrier_global[0]);

	if (current->config->worker_reverse_first_count || current->config->bidirectional)
		worker_barrier_destroy(&__worker_barrier_global[1]);

	worker_mutex_destroy(&__worker_mutex_global);
	worker_mutex_destroy(&__worker_mutex_cond);
	worker_cond_destroy(&__worker_cond_global);
}

#else

void worker_init(void) {
	return;
}

void worker_destroy(void) {
	return;
}

#endif
