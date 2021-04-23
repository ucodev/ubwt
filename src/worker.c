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

#include <pthread.h>

#include "current.h"
#include "error.h"
#include "worker.h"

ubwt_worker_barrier_t __worker_barrier_global[2]; /* 0: straight, 1: reverse */
ubwt_worker_mutex_t   __worker_mutex_global;

static void *_worker_task_init(void *arg) {
	ubwt_worker_task_t *t = arg;

	current_fork();

	switch (t->type) {
		case UBWT_WORKER_TASK_TYPE_INT: {
			t->fi(t->vi);
		} break;

		case UBWT_WORKER_TASK_TYPE_VOID: {
			t->fv(t->vv);
		} break;

		default: error_abort(__FILE__, __LINE__, "_worker_task_init");
	}

	return NULL;
}

void worker_exit(void) {
	int retval = 0;

	current_join(pthread_self());

	pthread_exit((void *) &retval);
}

void worker_create(ubwt_worker_task_t *t) {
	pthread_t tid;

	if (pthread_create(&tid, NULL, _worker_task_init, t)) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_WORKER_CREATE_FAILED, "worker_create(): pthread_create()");
		error_no_return();
	}
}

void worker_wait(ubwt_worker_barrier_t *barrier) {
	if (pthread_barrier_wait(barrier)) error_abort(__FILE__, __LINE__, "pthread_barrier_wait");
}

void worker_lock(ubwt_worker_mutex_t *mutex) {
	if (pthread_mutex_lock(mutex)) error_abort(__FILE__, __LINE__, "pthread_mutex_lock");
}

void worker_trylock(ubwt_worker_mutex_t *mutex) {
	if (pthread_mutex_trylock(mutex)) error_abort(__FILE__, __LINE__, "pthread_mutex_trylock");
}

void worker_unlock(ubwt_worker_mutex_t *mutex) {
	if (pthread_mutex_unlock(mutex)) error_abort(__FILE__, __LINE__, "pthread_mutex_unlock");
}

ubwt_worker_t worker_self(void) {
	return pthread_self();
}

int worker_im_cancelled(void) {
	return current->worker_cancel_requested;
}

int worker_is_joinable(ubwt_worker_t tid) {
	struct ubwt_current *c = current_get(tid);

	return c->worker_cancel_completed;
}

void worker_cancel(ubwt_worker_t tid) {
	struct ubwt_current *c = current_get(tid);

	c->worker_cancel_requested = 1;

	if (pthread_cancel(tid)) error_abort(__FILE__, __LINE__, "pthread_cancel");
}

void worker_join(ubwt_worker_t tid) {
	if (pthread_join(tid, NULL)) error_abort(__FILE__, __LINE__, "pthread_join");
}

void worker_init(void) {
	if (pthread_barrier_init(&__worker_barrier_global[0], NULL, current->config->worker_straight_first_count))
		error_abort(__FILE__, __LINE__, "pthread_barrier_init");

	if (pthread_barrier_init(&__worker_barrier_global[1], NULL, current->config->worker_reverse_first_count))
		error_abort(__FILE__, __LINE__, "pthread_barrier_init");

	if (pthread_mutex_init(&__worker_mutex_global, NULL))
		error_abort(__FILE__, __LINE__, "pthread_mutex_init");
}

void worker_destroy(void) {
	pthread_barrier_destroy(&__worker_barrier_global[0]);
	pthread_barrier_destroy(&__worker_barrier_global[1]);
	pthread_mutex_destroy(&__worker_mutex_global);
}

#else

void worker_init(void) {
	return;
}

void worker_destroy(void) {
	return;
}

#endif
