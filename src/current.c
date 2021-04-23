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

#include "current.h"
#include "datetime.h"
#include "error.h"
#include "stage.h"

#ifdef UBWT_CONFIG_MULTI_THREADED
 #include <assert.h>
 #include <stdlib.h>
 #include <pthread.h>

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
			return c;
	} while ((c = c->next));

	error_abort(__FILE__, __LINE__, "current_get");

	return NULL;
}
#endif

void current_init(void) {
	memset(&__current, 0, sizeof(struct ubwt_current));

#ifdef UBWT_CONFIG_MULTI_THREADED
	__current.worker_id = worker_self();

	__current.worker_barrier_global = __worker_barrier_global;
	__current.worker_mutex_global = &__worker_mutex_global;
#endif

	__current.config = &__config;
	memset(__current.config, 0, sizeof(struct ubwt_config));

	stage_set(UBWT_STAGE_STATE_INIT_CURRENT, 0);

	__current.time_us = &__current_time_us;

	*__current.time_us = datetime_now_us();
}

void current_update(void) {
	*current->time_us = datetime_now_us();
}

#ifdef UBWT_CONFIG_MULTI_THREADED
void current_fork(void) {
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

	c->worker_id = worker_self();

	if (pthread_mutex_init(&c->worker_mutex_local, NULL)) error_abort(__FILE__, __LINE__, "pthread_mutex_init");

	c->prev = &__current;
	c->next = __current.next;
	__current.next = c;
}

void current_join(ubwt_worker_t worker_id) {
	struct ubwt_current *c = &__current;

	do {
		if (c->worker_id == worker_id)
			break;
	} while ((c = current->next));

	assert(c != NULL);

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

	memset(c, 0, sizeof(struct ubwt_current));

	free(c);
}
#endif

void current_destroy(void) {
	stage_set(UBWT_STAGE_STATE_DESTROY_CURRENT, 0);

	memset(__current.config, 0, sizeof(struct ubwt_config));
	memset(&__current, 0, sizeof(struct ubwt_current));
}

