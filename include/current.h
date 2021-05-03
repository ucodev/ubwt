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

#ifndef UBWT_CURRENT_H
#define UBWT_CURRENT_H

#include <stdint.h>

#include "config.h"

#ifdef UBWT_CONFIG_MULTI_THREADED
 #include <pthread.h>
#endif

#include "error.h"
#include "net.h"
#include "report.h"
#include "stage.h"
#include "process.h"
#include "talk.h"
#include "worker.h"

#ifdef UBWT_CONFIG_MULTI_THREADED
 struct ubwt_current *current_get(ubwt_worker_t worker_id);
 #define current current_get(worker_self())
#else
 extern struct ubwt_current *current;
#endif

struct ubwt_current {
	struct ubwt_config *config;
	struct ubwt_net net;
	uint64_t *time_us;

	ubwt_stage_t stage;
	ubwt_error_t error;

	struct ubwt_report report;
	struct ubwt_process process;
	struct ubwt_talk_context talk;
	struct ubwt_runtime *runtime;

#ifdef UBWT_CONFIG_MULTI_THREADED
	ubwt_worker_t worker_id;
	ubwt_worker_t remote_worker_id;

	ubwt_worker_barrier_t *worker_barrier_global;
	ubwt_worker_mutex_t *worker_mutex_global;
	ubwt_worker_mutex_t *worker_mutex_cond;
	ubwt_worker_cond_t *worker_cond_global;
	ubwt_worker_mutex_t worker_mutex_local;
	unsigned int *worker_forking;

	ubwt_worker_task_t *worker_task;

	uint32_t worker_flags;

	struct ubwt_current *next, *prev;
#endif
};

void current_init(void);
void current_update(void);
#ifdef UBWT_CONFIG_MULTI_THREADED
void current_forking_set(unsigned int status);
unsigned int current_forking_get(void);
void current_fork(ubwt_worker_task_t *t);
void current_running_set(void);
void current_running_unset(void);
void current_exit(void);
void current_join(pthread_t tid);
int current_children_has_flag(unsigned int flag, unsigned int count);
#endif
void current_destroy(void);

#endif

