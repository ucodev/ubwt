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
#include <stdint.h>
#include <inttypes.h>

#include "config.h"

#ifdef UBWT_CONFIG_DEBUG

#include "current.h"
#include "datetime.h"
#include "debug.h"
#include "net.h"
#include "stage.h"
#include "talk.h"

#ifdef UBWT_CONFIG_MULTI_THREADED
 #include <assert.h>
 #include "worker.h"
#endif

void debug_info_talk_op(ubwt_talk_ops_t op, const char *msg) {
	char time_str[UBWT_CONFIG_CTIME_SIZE] = { 0 };

	if (!current->config->debug)
		return;

	datetime_now_str(time_str);

#ifdef UBWT_CONFIG_MULTI_THREADED
	assert(current->worker_id == worker_self());

	fprintf(stderr, "[%s]: [L#%" PRIu64 " <-> R#%" PRIu64 "]: DEBUG => INFO => %s => TALK: %s => %s\n", time_str, (uint64_t) worker_self(), (uint64_t) current->remote_worker_id, net_im_connector() ? "connector" : "listener", talk_op_to_str(op), msg);
#else
	fprintf(stderr, "[%s]: DEBUG => INFO => %s => TALK: %s => %s\n", time_str, net_im_connector() ? "connector" : "listener", talk_op_to_str(op), msg);
#endif
}

void debug_info_talk_latency(uint64_t dt) {
	char time_str[UBWT_CONFIG_CTIME_SIZE] = { 0 };

	if (!current->config->debug)
		return;

	datetime_now_str(time_str);

#ifdef UBWT_CONFIG_MULTI_THREADED
	assert(current->worker_id == worker_self());

	fprintf(stderr, "[%s]: [L#%" PRIu64 "]: DEBUG => INFO => %s => TALK: LATENCY: %" PRIu64 " us\n", time_str, (uint64_t) worker_self(), net_im_connector() ? "connector" : "listener", dt);
#else
	fprintf(stderr, "[%s]: DEBUG => INFO => %s => TALK: LATENCY: %" PRIu64 " us\n", time_str, net_im_connector() ? "connector" : "listener", dt);
#endif
}

void debug_info_config_show(void) {
	char time_str[UBWT_CONFIG_CTIME_SIZE] = { 0 };

	if (!current->config->debug)
		return;

	datetime_now_str(time_str);

#ifdef UBWT_CONFIG_MULTI_THREADED
	assert(current->worker_id == worker_self());

	fprintf(stderr, "[%s]: [#%" PRIu64 "]: DEBUG => INFO => %s => CONFIG: Mode      : %s\n", time_str, (uint64_t) worker_self(), net_im_connector() ? "connector" : "listener", current->config->im_connector ? "connector" : "listener");
	fprintf(stderr, "[%s]: [#%" PRIu64 "]: DEBUG => INFO => %s => CONFIG: Address   : %s\n", time_str, (uint64_t) worker_self(), net_im_connector() ? "connector" : "listener", current->config->addr);
	fprintf(stderr, "[%s]: [#%" PRIu64 "]: DEBUG => INFO => %s => CONFIG: Port      : %s\n", time_str, (uint64_t) worker_self(), net_im_connector() ? "connector" : "listener", current->config->port);
	fprintf(stderr, "[%s]: [#%" PRIu64 "]: DEBUG => INFO => %s => CONFIG: Protocol  : %s\n", time_str, (uint64_t) worker_self(), net_im_connector() ? "connector" : "listener", current->config->net_l4_proto_name);
	fprintf(stderr, "[%s]: [#%" PRIu64 "]: DEBUG => INFO => %s => CONFIG: Time (us) : %" PRIu64 "\n", time_str, (uint64_t) worker_self(), net_im_connector() ? "connector" : "listener", *current->time_us);
#else
	fprintf(stderr, "[%s]: DEBUG => INFO => %s => CONFIG: Mode      : %s\n", time_str, net_im_connector() ? "connector" : "listener", current->config->im_connector ? "connector" : "listener");
	fprintf(stderr, "[%s]: DEBUG => INFO => %s => CONFIG: Address   : %s\n", time_str, net_im_connector() ? "connector" : "listener", current->config->addr);
	fprintf(stderr, "[%s]: DEBUG => INFO => %s => CONFIG: Port      : %s\n", time_str, net_im_connector() ? "connector" : "listener", current->config->port);
	fprintf(stderr, "[%s]: DEBUG => INFO => %s => CONFIG: Protocol  : %s\n", time_str, net_im_connector() ? "connector" : "listener", current->config->net_l4_proto_name);
	fprintf(stderr, "[%s]: DEBUG => INFO => %s => CONFIG: Time (us) : %" PRIu64 "\n", time_str, net_im_connector() ? "connector" : "listener", *current->time_us);
#endif
}

void debug_info_stage_show(void) {
	char time_str[UBWT_CONFIG_CTIME_SIZE] = { 0 };

	if (!current->config->debug)
		return;

	datetime_now_str(time_str);

#ifdef UBWT_CONFIG_MULTI_THREADED
	assert(current->worker_id == worker_self());

	fprintf(stderr, "[%s]: [#%" PRIu64 "]: DEBUG => INFO => %s => STAGE: %s, ITER: %zu\n", time_str, (uint64_t) worker_self(), net_im_connector() ? "connector" : "listener", stage_state_to_str(current->stage.state), current->stage.iter);
#else
	fprintf(stderr, "[%s]: DEBUG => INFO => %s => STAGE: %s, ITER: %zu\n", time_str, net_im_connector() ? "connector" : "listener", stage_state_to_str(current->stage.state), current->stage.iter);
#endif
}

void debug_info_report_latency(uint32_t dt) {
	char time_str[UBWT_CONFIG_CTIME_SIZE] = { 0 };

	if (!current->config->debug)
		return;

	datetime_now_str(time_str);

#ifdef UBWT_CONFIG_MULTI_THREADED
	assert(current->worker_id == worker_self());

	fprintf(stderr, "[%s]: [#%" PRIu64 "]: DEBUG => INFO => %s => REPORT: Latency: %" PRIu32 " usec(s)\n", time_str, (uint64_t) worker_self(), net_im_connector() ? "connector" : "listener", dt);
#else
	fprintf(stderr, "[%s]: DEBUG => INFO => %s => REPORT: Latency: %" PRIu32 " usec(s)\n", time_str, net_im_connector() ? "connector" : "listener", dt);
#endif
}

void debug_info_report_count(uint32_t count) {
	char time_str[UBWT_CONFIG_CTIME_SIZE] = { 0 };

	if (!current->config->debug)
		return;

	datetime_now_str(time_str);

#ifdef UBWT_CONFIG_MULTI_THREADED
	assert(current->worker_id == worker_self());

	fprintf(stderr, "[%s]: [#%" PRIu64 "]: DEBUG => INFO => %s => REPORT: Count: %" PRIu32 "\n", time_str, (uint64_t) worker_self(), net_im_connector() ? "connector" : "listener", count);
#else
	fprintf(stderr, "[%s]: DEBUG => INFO => %s => REPORT: Count: %" PRIu32 "\n", time_str, net_im_connector() ? "connector" : "listener", count);
#endif
}

void debug_info_report_connection(const char *src, const char *ip, uint16_t port) {
	char time_str[UBWT_CONFIG_CTIME_SIZE] = { 0 };

	if (!current->config->debug)
		return;

	datetime_now_str(time_str);

#ifdef UBWT_CONFIG_MULTI_THREADED
	assert(current->worker_id == worker_self());

	fprintf(stderr, "[%s]: [#%" PRIu64 "]: DEBUG => INFO => %s => REPORT: %s connection: %s:%" PRIu32 "\n", time_str, (uint64_t) worker_self(), net_im_connector() ? "connector" : "listener", src, ip, port);
#else
	fprintf(stderr, "[%s]: DEBUG => INFO => %s => REPORT: %s connection: %s:%" PRIu32 "\n", time_str, net_im_connector() ? "connector" : "listener", src, ip, port);
#endif
}

#endif
