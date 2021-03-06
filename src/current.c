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

#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "config.h"

#include "bitops.h"
#include "current.h"
#include "datetime.h"
#include "error.h"
#include "runtime.h"
#include "stage.h"

#ifdef UBWT_CONFIG_MULTI_THREADED
 #include <assert.h>
 #include <stdlib.h>
 #include <unistd.h>

 #include "net.h"
 #include "worker.h"
#endif

uint64_t __current_time_us_start;
uint64_t __current_time_us_now;
struct ubwt_current __current;

#ifndef UBWT_CONFIG_MULTI_THREADED
struct ubwt_current *current = &__current;
#else
struct ubwt_current *current_get(ubwt_worker_t worker_id) {
#ifndef UBWT_CONFIG_PTHREAD_LOCAL_STORAGE
	struct ubwt_current *c = &__current;
#else
	struct ubwt_current *c = NULL;

	if (!current_im_main()) /* Don't use local thread storage for main thread */
		c = worker_getspecific(__worker_key_cptr);

	if (c) return c;

	c = &__current;
#endif

	do {
		if (c->worker_id == worker_id)
			break;
	} while ((c = c->next));

	assert(c != NULL);

#ifdef UBWT_CONFIG_PTHREAD_LOCAL_STORAGE
	if (!current_im_main()) /* Don't use local thread storage for main thread */
		worker_setspecific(__worker_key_cptr, c);
#endif

	return c;
}
#endif

void current_init(void) {
	memset(&__current, 0, sizeof(struct ubwt_current));

#ifdef UBWT_CONFIG_MULTI_THREADED
	__current.worker_id = worker_self();

	__current.worker_barrier_global = __worker_barrier_global;
	__current.worker_mutex_global = &__worker_mutex_global;
	__current.worker_mutex_cond = &__worker_mutex_cond;
	__current.worker_cond_global = &__worker_cond_global;
#ifdef UBWT_CONFIG_PTHREAD_LOCAL_STORAGE
	__current.worker_key_cptr = &__worker_key_cptr;
#endif
	__current.worker_forking = &__worker_forking;

	worker_mutex_init(&__current.worker_mutex_local);
#endif

	__current.config = &__config;
	memset(__current.config, 0, sizeof(struct ubwt_config));

	stage_set(UBWT_STAGE_STATE_INIT_CURRENT, 0);

	__current.time_us_start = &__current_time_us_start;
	__current.time_us_now = &__current_time_us_now;

	*__current.time_us_start = *__current.time_us_now = datetime_now_us();

	__current.runtime = &__runtime;
	__current.talk = __talk;
}

void current_update(void) {
	*current->time_us_now = datetime_now_us();
}

uint64_t current_time_now(void) {
	current_update();

	return *current->time_us_now;
}

uint64_t current_time_start(void) {
	return *current->time_us_start;
}

uint64_t current_time_elapsed(void) {
	return current_time_now() - current_time_start();
}

#ifdef UBWT_CONFIG_MULTI_THREADED
void current_forking_set(unsigned int status) {
	*current->worker_forking = status;
}

unsigned int current_forking_get(void) {
	return *current->worker_forking;
}

void current_fork(ubwt_worker_task_t *t) {
	struct ubwt_current *c = NULL;

	if (!(c = malloc(sizeof(struct ubwt_current)))) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CURRENT_FORK_FAILED, "current_fork(): malloc()");
		error_no_return();
	}

	memset(c, 0, sizeof(struct ubwt_current));

	/* Always fork from HEAD */

	c->config = __current.config;
	c->time_us_now = __current.time_us_now;
	c->time_us_start = __current.time_us_start;
	c->runtime = __current.runtime;
	c->talk = __current.talk;

	memcpy(&c->stage, &__current.stage, sizeof(ubwt_stage_t));

	memcpy(&c->net, &__current.net, sizeof(struct ubwt_net));

	if (__current.config->im_connector) {
		if ((c->net.fd = socket(c->net.domain, c->net.type, c->net.protocol)) < 0)
			error_abort(__FILE__, __LINE__, "socket");

		if (net_timeout_set(c->net.fd, c->config->net_timeout_default) < 0)
			error_abort(__FILE__, __LINE__, "net_timeout_set");
	}

	c->worker_id = worker_self();
	c->worker_task = t;
	c->worker_barrier_global = __worker_barrier_global;
	c->worker_mutex_global = &__worker_mutex_global;
	c->worker_mutex_cond = &__worker_mutex_cond;
	c->worker_cond_global = &__worker_cond_global;
#ifdef UBWT_CONFIG_PTHREAD_LOCAL_STORAGE
	c->worker_key_cptr = &__worker_key_cptr;
#endif
	c->worker_forking = &__worker_forking;

	worker_mutex_init(&c->worker_mutex_local);

	worker_mutex_lock(&c->worker_mutex_local);

	bit_set(&c->worker_flags, UBWT_WORKER_FLAG_TYPE_CHILD);
	bit_set(&c->worker_flags, UBWT_WORKER_FLAG_TASK_READY);

	worker_mutex_unlock(&c->worker_mutex_local);

	worker_mutex_lock(c->worker_mutex_global);

	c->prev = &__current;
	c->next = __current.next;

	if (c->next)
		c->next->prev = c;

	__current.next = c;

	worker_mutex_unlock(c->worker_mutex_global);

	current_update();
}

void current_running_set(void) {
	worker_mutex_lock(&current->worker_mutex_local);

	bit_set(&current->worker_flags, UBWT_WORKER_FLAG_TASK_RUNNING);

	worker_mutex_unlock(&current->worker_mutex_local);

	worker_cond_signal(current->worker_cond_global);

	current_update();
}

void current_running_unset(void) {
	worker_mutex_lock(&current->worker_mutex_local);

	bit_clear(&current->worker_flags, UBWT_WORKER_FLAG_TASK_RUNNING);

	worker_mutex_unlock(&current->worker_mutex_local);

	worker_cond_signal(current->worker_cond_global);

	current_update();
}

void current_exit(void) {
	worker_mutex_lock(&current->worker_mutex_local);

	bit_clear(&current->worker_flags, UBWT_WORKER_FLAG_TASK_RUNNING);
	bit_set(&current->worker_flags, UBWT_WORKER_FLAG_TASK_JOINABLE);

	worker_cond_signal(current->worker_cond_global);

	current_update();

	worker_mutex_unlock(&current->worker_mutex_local);
}

void current_join(ubwt_worker_t worker_id) {
	struct ubwt_current *c = &__current;

	do {
		if (c->worker_id == worker_id)
			break;
	} while ((c = c->next));

	assert(c != NULL); /* Task must exist */

	assert(c->worker_id != worker_self()); /* Cannot join to itself */

	worker_mutex_lock(&c->worker_mutex_local);

	assert(!bit_test(&c->worker_flags, UBWT_WORKER_FLAG_TASK_RUNNING)); /* Task should be in a stopped state */
	assert(bit_test(&c->worker_flags, UBWT_WORKER_FLAG_TASK_JOINABLE)); /* Task must be in a joinable state */

	worker_mutex_unlock(&c->worker_mutex_local);

	worker_mutex_lock(c->worker_mutex_global);

	if (c->prev && c->next) {
		c->prev->next = c->next;
		c->next->prev = c->prev;
	} else if (c->prev) {
		/* tail */
		c->prev->next = NULL;
	} else {
		/* head - head is static and not manageable - if we are here, there is a problem in the LL management */
		error_abort(__FILE__, __LINE__, "current_join");
	}

	worker_mutex_unlock(c->worker_mutex_global);

	worker_mutex_destroy(&c->worker_mutex_local);

	memset(c->worker_task, 0, sizeof(ubwt_worker_task_t));
	free(c->worker_task);

#ifdef COMPILE_WIN32
	closesocket(c->net.fd);
#else
	close(c->net.fd);
#endif

	memset(c, 0, sizeof(struct ubwt_current));
	free(c);
}

int current_children_has_flag(unsigned int flag, unsigned int count) {
	struct ubwt_current *c = &__current;

	assert(count > 0);

	do {
		/* Ignore non-child threads */
		if (!bit_test(&c->worker_flags, UBWT_WORKER_FLAG_TYPE_CHILD))
			continue;

		if (bit_test(&c->worker_flags, flag))
			count --;
	} while ((c = c->next) && count);

	return !count;
}
#endif

void current_destroy(void) {
	stage_set(UBWT_STAGE_STATE_DESTROY_CURRENT, 0);

	memset(__current.config, 0, sizeof(struct ubwt_config));
	memset(&__current, 0, sizeof(struct ubwt_current));
}

