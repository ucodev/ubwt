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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "config.h"
#include "current.h"
#include "process.h"
#include "runtime.h"
#include "worker.h"

struct ubwt_runtime __runtime = {
	0,	/* progress_tick */
	"-\\|/"	/* progress_rotary */
};

static void _runtime_display_progress(const char *section) {
	fprintf(stdout, "\r [%c] %s...", __runtime.progress_rotary[__runtime.progress_tick ++ % sizeof(__runtime.progress_tick)], section);
	fflush(stdout);
}

static void _runtime_display_done(const char *section) {
	fprintf(stderr, "\r [*] %s...\n", section);
}

#ifdef UBWT_CONFIG_MULTI_THREADED
static void _runtime_wait_worker_flag(const char *section, unsigned int flag) {
	struct timespec abstime = { 0, 0 };
	_runtime_display_progress(section);

	worker_mutex_lock(current->worker_mutex_cond);

	while (!current_children_has_flag(flag)) {
		clock_gettime(CLOCK_REALTIME, &abstime);

		abstime.tv_nsec += 125000000;

		if (abstime.tv_nsec > 1000000000) {
			abstime.tv_sec += 1;
			abstime.tv_nsec -= 1000000000;
		}

		_runtime_display_progress(section);

		worker_cond_timedwait(current->worker_cond_global, current->worker_mutex_cond, &abstime);
	}

	worker_mutex_unlock(current->worker_mutex_cond);

	_runtime_display_done(section);
}
#endif

void runtime_do(void) {
#ifdef UBWT_CONFIG_MULTI_THREADED
	ubwt_worker_t tid;
	ubwt_worker_task_t *t = NULL;

	if (!(t = malloc(sizeof(ubwt_worker_task_t)))) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_WORKER_CREATE_FAILED, "runtime_do(): malloc()");
		error_no_return();
	}

	t->type = UBWT_WORKER_TASK_TYPE_NVR_INT;
	t->fi = process_run;
	t->vi = 0;

	tid = worker_task_create(t);

	_runtime_wait_worker_flag("Initializing workers", UBWT_WORKER_FLAG_TASK_READY);
	_runtime_wait_worker_flag("Waiting for communication", UBWT_WORKER_FLAG_TASK_RUNNING);
	_runtime_wait_worker_flag("Running bandwidth test", UBWT_WORKER_FLAG_TASK_JOINABLE);
#else
	process_run(0 /* 0: straight first, 1: reverse first */);
#endif

	process_report();

#ifdef UBWT_CONFIG_MULTI_THREADED
	current_join(tid);
#endif
}

