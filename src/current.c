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

 #include "worker.h"
#endif

uint64_t __current_time_us;
struct ubwt_current __current;

#ifndef UBWT_CONFIG_MULTI_THREADED
struct ubwt_current *current = &__current;
#else
struct ubwt_current *current_get(ubwt_worker_t worker_id) {
	struct ubwt_current *c = &__current;

	do {
		if (c->worker_id == worker_id)
			break;
	} while ((c = c->next));

	assert(c != NULL);

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

	worker_mutex_init(&__current.worker_mutex_local);
#endif

	__current.config = &__config;
	memset(__current.config, 0, sizeof(struct ubwt_config));

	stage_set(UBWT_STAGE_STATE_INIT_CURRENT, 0);

	__current.time_us = &__current_time_us;

	*__current.time_us = datetime_now_us();

	__current.runtime = &__runtime;
}

void current_update(void) {
	*current->time_us = datetime_now_us();
}

#ifdef UBWT_CONFIG_MULTI_THREADED
void current_fork(ubwt_worker_task_t *t) {
	struct ubwt_current *c = NULL;

	if (!(c = malloc(sizeof(struct ubwt_current)))) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CURRENT_FORK_FAILED, "current_fork(): malloc()");
		error_no_return();
	}

	memset(c, 0, sizeof(struct ubwt_current));

	/* Always fork from HEAD */

	memcpy(&c->net, &__current.net, sizeof(struct ubwt_net));
	c->config = __current.config;
	c->time_us = __current.time_us;
	c->runtime = __current.runtime;

	c->worker_id = worker_self();
	c->worker_task = t;
	c->worker_barrier_global = __worker_barrier_global;
	c->worker_mutex_global = &__worker_mutex_global;
	c->worker_mutex_cond = &__worker_mutex_cond;
	c->worker_cond_global = &__worker_cond_global;

	worker_mutex_init(&c->worker_mutex_local);

	worker_mutex_lock(&c->worker_mutex_local);

	bit_set(&c->worker_flags, UBWT_WORKER_FLAG_TYPE_CHILD);
	bit_set(&c->worker_flags, UBWT_WORKER_FLAG_TASK_READY);

	worker_mutex_unlock(&c->worker_mutex_local);

	worker_mutex_lock(c->worker_mutex_global);

	c->prev = &__current;
	c->next = __current.next;
	__current.next = c;

	worker_mutex_unlock(c->worker_mutex_global);

	worker_cond_signal(c->worker_cond_global);

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

	worker_mutex_unlock(&current->worker_mutex_local);

	worker_cond_signal(current->worker_cond_global);

	current_update();
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
	} else if (c->next) {
		/* head */
		c->next->prev = NULL;
	}

	worker_mutex_unlock(c->worker_mutex_global);

	worker_mutex_destroy(&c->worker_mutex_local);

	memset(c->worker_task, 0, sizeof(ubwt_worker_task_t));
	free(c->worker_task);

	memset(c, 0, sizeof(struct ubwt_current));
	free(c);

	worker_join(worker_id);
}

int current_children_has_flag(unsigned int flag) {
	struct ubwt_current *c = &__current;

	do {
		/* Ignore non-child threads */
		if (!bit_test(&c->worker_flags, UBWT_WORKER_FLAG_TYPE_CHILD))
			continue;

		if (!bit_test(&c->worker_flags, flag))
			return 0;
	} while ((c = c->next));

	return 1;
}
#endif

void current_destroy(void) {
	stage_set(UBWT_STAGE_STATE_DESTROY_CURRENT, 0);

	memset(__current.config, 0, sizeof(struct ubwt_config));
	memset(&__current, 0, sizeof(struct ubwt_current));
}

