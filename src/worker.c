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

#include "current.h"
#include "error.h"
#include "worker.h"

ubwt_worker_barrier_t __worker_barrier_straight_first;
ubwt_worker_barrier_t __worker_barrier_reverse_first;
ubwt_worker_mutex_t   __worker_mutex_global;

static void *_worker_process_init(void *f) {
	void (*w) (void) = (void (*) (void)) f;

	current_fork();

	w();

	return NULL;
}

void worker_exit(void) {
	int retval = 0;

	current_join(pthread_self());

	pthread_exit((void *) &retval);
}

void worker_create(void *f) {
	pthread_t tid;

	if (pthread_create(&tid, NULL, _worker_process_init, f)) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_WORKER_CREATE_FAILED, "worker_create(): pthread_create()");
		error_no_return();
	}
}

void worker_wait(ubwt_worker_barrier_t *barrier) {
	if (pthread_barrier_wait(barrier)) abort();
}

void worker_lock(ubwt_worker_mutex_t *mutex) {
	if (pthread_mutex_lock(mutex)) abort();
}

void worker_trylock(ubwt_worker_mutex_t *mutex) {
	if (pthread_mutex_trylock(mutex)) abort();
}

void worker_unlock(ubwt_worker_mutex_t *mutex) {
	if (pthread_mutex_unlock(mutex)) abort();
}

ubwt_worker_t worker_self(void) {
	return pthread_self();
}

int worker_im_cancelled(void) {
	return current->worker_cancel_requested;
}

void worker_cancel(ubwt_worker_t tid) {
	struct ubwt_current *c = current_get(tid);

	c->worker_cancel_requested = 1;

	if (pthread_cancel(tid)) abort();
}

void worker_join(ubwt_worker_t tid) {
	if (pthread_join(tid, NULL)) abort();
}

void worker_init(void) {
	if (pthread_barrier_init(&__worker_barrier_straight_first, NULL, current->config->worker_straight_first_count))
		abort();

	if (pthread_barrier_init(&__worker_barrier_reverse_first, NULL, current->config->worker_reverse_first_count))
		abort();

	if (pthread_mutex_init(&__worker_mutex_global, NULL))
		abort();
}

void worker_destroy(void) {
	pthread_barrier_destroy(&__worker_barrier_straight_first);
	pthread_barrier_destroy(&__worker_barrier_reverse_first);
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
