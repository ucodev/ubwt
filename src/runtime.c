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
#include <assert.h>

#include "config.h"
#include "current.h"
#include "process.h"
#include "runtime.h"
#include "worker.h"

struct ubwt_runtime __runtime = {
	0,	/* progress_tick */
	"-\\|/"	/* progress_rotary */
};

static void _runtime_write_pid_file(void) {
	FILE *fp = NULL;

	if (current->config->pid_file) {
		if (!(fp = fopen(current->config->pid_file, "w+")))
			error_abort(__FILE__, __LINE__, "_runtime_write_pid_file");

		fprintf(fp, "%u", getpid());

		fclose(fp);
	}
}

#ifdef UBWT_CONFIG_MULTI_THREADED
static void _runtime_display_progress(const char *section) {
	fprintf(stdout, "\r [%c] %s...", __runtime.progress_rotary[__runtime.progress_tick ++ % sizeof(__runtime.progress_tick)], section);
	fflush(stdout);
}

static void _runtime_display_done(const char *section) {
	fprintf(stdout, "\r [*] %s...\n", section);
}

static void _runtime_wait_worker_flag(const char *section, unsigned int flag, unsigned int count) {
	struct timespec abstime = { 0, 0 };

	_runtime_display_progress(section);

	worker_mutex_lock(current->worker_mutex_cond);

	while (!worker_task_has_flag(flag, count)) { /* Check if count children workers have 'flag' set */
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
	unsigned int i = 0, j = 0;
	ubwt_worker_t *tid = NULL;
	ubwt_worker_task_t **t = NULL;

	assert((current->config->worker_straight_first_count + current->config->worker_reverse_first_count) == current->config->worker_count);

	_runtime_write_pid_file();

	if (!(t = malloc(sizeof(ubwt_worker_task_t *) * (current->config->worker_straight_first_count + current->config->worker_reverse_first_count)))) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_WORKER_CREATE_FAILED, "runtime_do(): t = malloc()");
		error_no_return();
	}

	for (i = 0; i < (current->config->worker_straight_first_count + current->config->worker_reverse_first_count); i ++) {
		if (!(t[i] = malloc(sizeof(ubwt_worker_task_t)))) {
			error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_WORKER_CREATE_FAILED, "runtime_do(): t[i] = malloc()");
			error_no_return();
		}
	}

	if (!(tid = malloc(sizeof(ubwt_worker_t) * (current->config->worker_straight_first_count + current->config->worker_reverse_first_count)))) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_WORKER_CREATE_FAILED, "runtime_do(): tid = malloc()");
		error_no_return();
	}

	/* Initialize straight first workers */
	for (i = 0; i < current->config->worker_straight_first_count; i ++) {
		t[i]->type = UBWT_WORKER_TASK_TYPE_NVR_INT;
		t[i]->fi = process_run;
		t[i]->vi = 0;

		tid[i] = worker_task_create(t[i]);
	}

	if (i > 0) {
		_runtime_wait_worker_flag("Initializing local regular workers", UBWT_WORKER_FLAG_TASK_READY, current->config->asynchronous ? current->config->worker_count / 2 : current->config->worker_count);
		_runtime_wait_worker_flag("Waiting for remote regular workers", UBWT_WORKER_FLAG_TASK_RUNNING, current->config->asynchronous ? current->config->worker_count / 2 : current->config->worker_count);
	}

	/* Initialize reverse first workers */
	for (j = i; i < (current->config->worker_straight_first_count + current->config->worker_reverse_first_count); i ++) {
		t[i]->type = UBWT_WORKER_TASK_TYPE_NVR_INT;
		t[i]->fi = process_run;
		t[i]->vi = 1;

		tid[i] = worker_task_create(t[i]);
	}

	if (i > j) {
		_runtime_wait_worker_flag("Initializing local reverse workers", UBWT_WORKER_FLAG_TASK_READY, current->config->worker_count);
		_runtime_wait_worker_flag("Waiting for remote reverse workers", UBWT_WORKER_FLAG_TASK_RUNNING, current->config->worker_count);
	}

	if (i > 0) {
		_runtime_wait_worker_flag("Running bandwidth test", UBWT_WORKER_FLAG_TASK_JOINABLE, current->config->worker_count);

		process_report();
	} else {
		puts("Nothing to do...");
	}
#else
	_runtime_write_pid_file();

	puts("Running bandwidth test - this may take a while...");

	process_run(current->config->reverse_first /* 0: straight first, 1: reverse first */);

	process_report();
#endif


#ifdef UBWT_CONFIG_MULTI_THREADED
	for (i = 0; i < (current->config->worker_straight_first_count + current->config->worker_reverse_first_count); i ++) {
		worker_task_join(tid[i]);
	}

	free(tid);
	free(t);
#endif
}

