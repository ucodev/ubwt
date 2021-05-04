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

#include <assert.h>
#include <stdlib.h>
#include <pthread.h>

#include "bitops.h"
#include "current.h"
#include "error.h"
#include "worker.h"

ubwt_worker_barrier_t __worker_barrier_global[3]; /* 0: straight, 1: reverse */
ubwt_worker_mutex_t   __worker_mutex_global; /* used by 'current' handlers */
ubwt_worker_mutex_t   __worker_mutex_cond;
ubwt_worker_cond_t    __worker_cond_global;
unsigned int          __worker_forking = 0;

static void *_worker_task_init(void *arg) {
	ubwt_worker_task_t *t = arg;

	current_fork(t);

	current_forking_set(0);

	worker_cond_signal(current->worker_cond_global);

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

int worker_task_has_flag(unsigned int flag, unsigned int count) {
	return current_children_has_flag(flag, count);
}

void worker_task_join(ubwt_worker_t tid) {
	current_join(tid);

	if (pthread_join(tid, NULL)) error_abort(__FILE__, __LINE__, "pthread_join");
}

ubwt_worker_t worker_task_create(ubwt_worker_task_t *t) {
	ubwt_worker_t tid;

	current_forking_set(1);

	if (pthread_create(&tid, NULL, _worker_task_init, t)) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_WORKER_CREATE_FAILED, "worker_task_create(): pthread_create()");
		error_no_return();
	}

	worker_mutex_lock(current->worker_mutex_cond);

	while (current_forking_get())
		worker_cond_wait(current->worker_cond_global, current->worker_mutex_cond);

	worker_mutex_unlock(current->worker_mutex_cond);

	return tid;
}

void worker_barrier_init(ubwt_worker_barrier_t *barrier, unsigned count) {
#ifdef UBWT_CONFIG_CUSTOM_PTHREAD_BARRIER
	memset(barrier, 0, sizeof(ubwt_worker_barrier_t));

	barrier->total = barrier->count = count;

	worker_mutex_init(&barrier->mutex[0]);
	worker_mutex_init(&barrier->mutex[1]);
	worker_cond_init(&barrier->cond[0]);
	worker_cond_init(&barrier->cond[1]);
#else
	if (pthread_barrier_init(barrier, NULL, count)) error_abort(__FILE__, __LINE__, "pthread_barrier_init");
#endif
}

int worker_barrier_wait(ubwt_worker_barrier_t *barrier) {
#ifdef UBWT_CONFIG_CUSTOM_PTHREAD_BARRIER
	int serial = 0;

	worker_mutex_lock(&barrier->mutex[0]);

	/* Wait if this barrier is still in use by a previous call */
	while (barrier->count != barrier->total)
		worker_cond_wait(&barrier->cond[0], &barrier->mutex[0]);

	worker_mutex_lock(&barrier->mutex[1]);

	worker_mutex_unlock(&barrier->mutex[0]);

	barrier->waiting ++;

	/* Wait for all threads */
	while (barrier->waiting < barrier->count)
		worker_cond_wait(&barrier->cond[1], &barrier->mutex[1]);

	assert(barrier->count == barrier->waiting);

	/* First thread released, broadcasts the condition */
	if (barrier->count == barrier->total)
		worker_cond_broadcast(&barrier->cond[1]);

	barrier->count --;
	barrier->waiting --;

	/* Last thread released, resets the barrier */
	if (!barrier->count) {
		barrier->count = barrier->total;
		worker_cond_broadcast(&barrier->cond[0]);
		serial = 1;
	}

	worker_mutex_unlock(&barrier->mutex[1]);

	return serial;
#else
	return pthread_barrier_wait(barrier) == PTHREAD_BARRIER_SERIAL_THREAD;
#endif
}

void worker_barrier_destroy(ubwt_worker_barrier_t *barrier) {
#ifdef UBWT_CONFIG_CUSTOM_PTHREAD_BARRIER
	worker_mutex_destroy(&barrier->mutex[0]);
	worker_mutex_destroy(&barrier->mutex[1]);
	worker_cond_destroy(&barrier->cond[0]);
	worker_cond_destroy(&barrier->cond[1]);

	memset(barrier, 0, sizeof(ubwt_worker_barrier_t));
#else
	pthread_barrier_destroy(barrier);
#endif
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
	if (pthread_cond_broadcast(cond)) error_abort(__FILE__, __LINE__, "pthread_cond_broadcast");
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
	}

	if (current->config->worker_reverse_first_count && current->config->bidirectional) {
		worker_barrier_init(&__worker_barrier_global[1], current->config->worker_reverse_first_count);
	} else if (current->config->worker_reverse_first_count) {
		worker_barrier_init(&__worker_barrier_global[1], current->config->worker_reverse_first_count);
	} else if (current->config->bidirectional) {
		worker_barrier_init(&__worker_barrier_global[1], current->config->worker_count);
	}

	if (current->config->asynchronous)
		worker_barrier_init(&__worker_barrier_global[2], current->config->worker_count);

	worker_mutex_init(&__worker_mutex_global);
	worker_mutex_init(&__worker_mutex_cond);
	worker_cond_init(&__worker_cond_global);
}

void worker_destroy(void) {
	if (current->config->worker_straight_first_count || current->config->bidirectional)
		worker_barrier_destroy(&__worker_barrier_global[0]);

	if (current->config->worker_reverse_first_count || current->config->bidirectional)
		worker_barrier_destroy(&__worker_barrier_global[1]);

	if (current->config->asynchronous)
		worker_barrier_destroy(&__worker_barrier_global[2]);

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
