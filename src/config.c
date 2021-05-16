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
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>

#include "config.h"
#include "current.h"
#include "debug.h"
#include "error.h"
#include "net.h"
#include "stage.h"
#include "usage.h"

struct ubwt_config __config;

static void _config_sanity(void) {
	assert(strlen(current->config->port) <= 5 && atoi(current->config->port) >= 1 && (atoi(current->config->port) <= 65535));

	assert(current->config->net_mtu >= UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE);
	assert(current->config->net_timeout_default > 0 && current->config->net_timeout_default <= 65535);
	assert(current->config->net_timeout_talk_stream_end > 0 && current->config->net_timeout_talk_stream_end < current->config->net_timeout_default);
	assert(current->config->net_timeout_talk_stream_run > 0 && current->config->net_timeout_talk_stream_run < current->config->net_timeout_talk_stream_end);

	assert(current->config->net_l4_proto_value != 0);

	assert(current->config->process_reverse_delay > 0 && current->config->process_reverse_delay < current->config->net_timeout_talk_stream_end);

	//assert(current->config->talk_handshake_interval >= 0);
	assert(current->config->talk_handshake_iter > 0);

	assert(current->config->talk_count_current > 0);
	assert(current->config->talk_count_default > 0);
	assert(current->config->talk_count_max > 0);

	if (current->config->talk_count_mul)
		assert(current->config->talk_count_mul > 1 && current->config->talk_count_mul < UBWT_CONFIG_TALK_COUNT_MUL_MAX);

	assert(current->config->talk_payload_current_size >= UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE);
	assert(current->config->talk_payload_default_size >= UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE);
	assert(current->config->talk_payload_max_size >= UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE);

	assert(current->config->talk_payload_current_size > offsetof(ubwt_talk_payload_t, buf));
	assert(current->config->talk_payload_default_size > offsetof(ubwt_talk_payload_t, buf));
	assert(current->config->talk_payload_max_size > offsetof(ubwt_talk_payload_t, buf));
	assert(current->config->talk_payload_max_size >= current->config->talk_payload_current_size);
	assert(current->config->talk_payload_max_size >= current->config->talk_payload_default_size);

	assert(current->config->talk_stream_minimum_time > 0);

#ifdef UBWT_CONFIG_MULTI_THREADED
	assert(current->config->worker_count > 0 && current->config->worker_count <= 1024);

	if (current->config->asynchronous) {
		assert(!(current->config->worker_count & 1)); /* Asynchronous mode requires worker_count to be an even value */
		assert(!current->config->reverse_first); /* No reverse first in asynchronous mode - pointless */
	}

	if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_UDP) {
		/* Asynchronous testing is not available with UDP */
		assert(current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_UDP && !current->config->asynchronous);

		/* UDP testing must be performed with a single worker */
		assert(current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_UDP && current->config->worker_count == 1);
	}
#endif
}

static void _config_consistency(void) {
	if (current->config->net_timeout_talk_stream_run >= current->config->net_timeout_talk_stream_end) {
		errno = EINVAL;

		error_handler(
			UBWT_ERROR_LEVEL_FATAL,
			UBWT_ERROR_TYPE_CONFIG_CONSISTENCY_FAILED,
			"_config_consistency(): "
			"The specified stream run timeout value (%" PRIu16 ") "
			"cannot be equal or greater than the stream end timeout value (%" PRIu16 ")",
			current->config->net_timeout_talk_stream_run,
			current->config->net_timeout_talk_stream_end
		);

		error_no_return();
	}

	if (current->config->net_timeout_talk_stream_end >= current->config->net_timeout_default) {
		errno = EINVAL;

		error_handler(
			UBWT_ERROR_LEVEL_FATAL,
			UBWT_ERROR_TYPE_CONFIG_CONSISTENCY_FAILED,
			"_config_consistency(): "
			"The specified stream end timeout value (%" PRIu16 ") "
			"cannot be equal or greater than the connection timeout value (%" PRIu16 ")",
			current->config->net_timeout_talk_stream_end,
			current->config->net_timeout_default
		);

		error_no_return();
	}

	if (current->config->process_reverse_delay >= current->config->net_timeout_talk_stream_end) {
		errno = EINVAL;

		error_handler(
			UBWT_ERROR_LEVEL_FATAL,
			UBWT_ERROR_TYPE_CONFIG_CONSISTENCY_FAILED,
			"_config_consistency(): "
			"The specified delay time for process reverse (%" PRIu16 ") "
			"cannot be equal or greater than the stream end timeout value (%" PRIu16 ")",
			current->config->process_reverse_delay,
			current->config->net_timeout_talk_stream_end
		);

		error_no_return();
	}
}

static void _config_cmdopt_process(int argc, char *const *argv) {
	int opt = 0;
	const char *flags =
#ifdef UBWT_CONFIG_MULTI_THREADED
		"A"
#endif
#ifdef UBWT_CONFIG_DEBUG
		"d:D"
#endif
		"bFhI:j:l:m:M:N:o:"
#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
		"O:"
#endif
#ifndef UBWT_CONFIG_NET_NO_UDP
		"p:"
#endif
		"P:r:Rs:t:"
#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
		"T:"
#endif
		"v"
#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
		"w:"
#endif
#ifdef UBWT_CONFIG_MULTI_THREADED
		"W:"
#endif
		;

	while ((opt = getopt(argc, argv, flags)) != -1) {
		usage_check_optarg(opt, optarg);

		switch (opt) {
#ifdef UBWT_CONFIG_MULTI_THREADED
			case 'A': {
				current->config->asynchronous = 1;
				current->config->bidirectional = 1;
			} break;
#endif
#ifdef UBWT_CONFIG_DEBUG
			case 'd': {
				assert(strlen(optarg) > 0);
				current->config->debug_file = malloc(strlen(optarg) + 1);
				assert(current->config->debug_file);
				strcpy(current->config->debug_file, optarg);
				current->config->debug = 1;
				debug_update();
			} break;

			case 'D': {
				current->config->debug = 1;
				debug_update();
			} break;
#endif
			case 'b': {
				current->config->bidirectional = 1;
			} break;

			case 'F': {
				current->config->report_full = 1;
			} break;

			case 'h': {
				usage_show(argv, 1);
				error_no_return();
			} break;

			case 'I': {
				assert(atoi(optarg) >= 0 && atoi(optarg) < 65536);
				current->config->talk_handshake_interval = (uint16_t) atoi(optarg);
			} break;

			case 'j': {
				assert(strlen(optarg) > 0);
				current->config->report_json_file = malloc(strlen(optarg) + 1);
				assert(current->config->report_json_file);
				strcpy(current->config->report_json_file, optarg);
			} break;

			case 'm': {
				assert(atoi(optarg) >= UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE && atoi(optarg) < 65536);
				current->config->net_mtu = (uint16_t) atoi(optarg);
			} break;

			case 'M': {
				assert(atoi(optarg) > 1 && atoi(optarg) < UBWT_CONFIG_TALK_COUNT_MUL_MAX);
				current->config->talk_count_mul = atoi(optarg);
			} break;

			case 'N': {
				assert(atoi(optarg) > 0 && atoi(optarg) < 65536);
				current->config->talk_handshake_iter = (uint16_t) atoi(optarg);
			} break;

			case 'o': {
				assert(atoi(optarg) > 0 && atoi(optarg) < 65536);
				current->config->process_reverse_delay = (uint16_t) atoi(optarg);
				current->config->bidirectional = 1;
			} break;

#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
			case 'O': {
				assert(atoi(optarg) > 0 && atoi(optarg) < 65536);
				current->config->net_timeout_talk_stream_end = (uint16_t) atoi(optarg);
			} break;
#endif

			case 'p': {
				assert(strlen(optarg) < sizeof(current->config->net_l4_proto_name));
				strncpy(current->config->net_l4_proto_name, optarg, sizeof(current->config->net_l4_proto_name) - 1);
				current->config->net_l4_proto_name[sizeof(current->config->net_l4_proto_name) - 1] = 0;

				if (!strcmp(optarg, "tcp")) {
					current->config->net_l4_proto_value = UBWT_NET_PROTO_L4_TCP;
#ifndef UBWT_CONFIG_NET_NO_UDP
				} else if (!strcmp(optarg, "udp")) {
					current->config->net_l4_proto_value = UBWT_NET_PROTO_L4_UDP;
#endif
				} else {
					usage_show(argv, 0);
					error_no_return();
				}
			} break;

			case 'P': {
				strncpy(current->config->port, optarg, sizeof(current->config->port) - 1);
				current->config->port[sizeof(current->config->port) - 1] = 0;
			} break;

			case 'r': {
				assert(strlen(optarg) > 0);
				current->config->pid_file = malloc(strlen(optarg) + 1);
				assert(current->config->pid_file);
				strcpy(current->config->pid_file, optarg);
			} break;

			case 'R': {
				current->config->reverse_first = 1;
			} break;

			case 's': {
				assert(atoi(optarg) >= UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE && atoi(optarg) < 65536);
				current->config->talk_payload_default_size = (uint16_t) atoi(optarg);
			} break;

			case 't': {
				assert(atoi(optarg) > 0 && atoi(optarg) < 65536);
				current->config->talk_stream_minimum_time = (uint16_t) atoi(optarg);
			} break;

#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
			case 'T': {
				assert(atoi(optarg) > 0 && atoi(optarg) < UBWT_CONFIG_NET_TIMEOUT_TALK_STREAM_END);
				current->config->net_timeout_talk_stream_run = (uint16_t) atoi(optarg);
			} break;
#endif
			case 'v': {
				usage_version();
				error_no_return();
			} break;

#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
			case 'w': {
				assert(atoi(optarg) > 0 && atoi(optarg) < 65536);
				current->config->net_timeout_default = (uint16_t) atoi(optarg);
			} break;
#endif

#ifdef UBWT_CONFIG_MULTI_THREADED
			case 'W': {
				assert(atoi(optarg) > 0 && atoi(optarg) <= 1024);
				current->config->worker_count = (uint16_t) atoi(optarg);
			} break;
#endif

			default: {
				errno = EINVAL;
				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPT_INVALID, "_config_cmdopt_process()");
				error_no_return();
			}
		}
	}

#ifdef UBWT_CONFIG_DEBUG
	if (current->config->debug)
		current->config->error_log_min_level = UBWT_ERROR_LEVEL_INFO;
#endif

	if ((argc - optind) != 2) {
		usage_show(argv, 0);
		error_no_return();
	}

	if (!strcmp(argv[optind], "connector")) {
		current->config->im_connector = 1;
	} else if (!strcmp(argv[optind], "listener")) {
		current->config->im_listener = 1;
	} else {
		errno = EINVAL;
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_1_INVALID, "config_init()");
		error_no_return();
	}

	optind ++;

	assert(strlen(argv[optind]) < sizeof(current->config->addr));

	memset(current->config->addr, 0, sizeof(current->config->addr));
	strncpy(current->config->addr, argv[optind], sizeof(current->config->addr) - 1);
	current->config->addr[sizeof(current->config->addr) - 1] = 0;
}

void config_init(int argc, char *const *argv) {
	stage_set(UBWT_STAGE_STATE_INIT_CONFIG, 0);

	memset(current->config, 0, sizeof(struct ubwt_config));

	current->config->error_log_min_level = UBWT_ERROR_LEVEL_CRITICAL;

	strncpy(current->config->port, UBWT_CONFIG_PORT_DEFAULT, sizeof(current->config->port) - 1);
	current->config->port[sizeof(current->config->port) - 1] = 0;

	current->config->net_timeout_default = UBWT_CONFIG_NET_TIMEOUT_DEFAULT;
	current->config->net_timeout_talk_stream_run = UBWT_CONFIG_NET_TIMEOUT_TALK_STREAM_RUN;
	current->config->net_timeout_talk_stream_end = UBWT_CONFIG_NET_TIMEOUT_TALK_STREAM_END;

	current->config->net_mtu = UBWT_CONFIG_NET_MTU;
#if defined(UBWT_CONFIG_NET_REUSE_ADDRESS) && defined(SO_REUSEADDR)
	current->config->net_reuseaddr = 1;
#else
	current->config->net_reuseaddr = 0;
#endif
#if defined(UBWT_CONFIG_NET_REUSE_PORT) && defined(SO_REUSEPORT)
	current->config->net_reuseport = 1;
#else
	current->config->net_reuseport = 0;
#endif
	current->config->net_l2_hdr_size = UBWT_CONFIG_NET_L2_HEADER_SIZE;
	current->config->net_l3_ipv4_hdr_size = UBWT_CONFIG_NET_L3_IPV4_HEADER_SIZE;
	current->config->net_l3_ipv6_hdr_size = UBWT_CONFIG_NET_L3_IPV6_HEADER_SIZE;
	strncpy(current->config->net_l4_proto_name, UBWT_CONFIG_NET_L4_PROTO_DEFAULT, sizeof(current->config->net_l4_proto_name) - 1);
	current->config->net_l4_proto_name[sizeof(current->config->net_l4_proto_name) - 1] = 0;

	if (!strcmp(UBWT_CONFIG_NET_L4_PROTO_DEFAULT, "tcp")) {
		current->config->net_l4_proto_value = UBWT_NET_PROTO_L4_TCP;
		current->config->net_l4_hdr_size = UBWT_CONFIG_NET_L4_TCP_HEADER_SIZE;
	} else if (!strcmp(UBWT_CONFIG_NET_L4_PROTO_DEFAULT, "udp")) {
		current->config->net_l4_proto_value = UBWT_NET_PROTO_L4_UDP;
		current->config->net_l4_hdr_size = UBWT_CONFIG_NET_L4_UDP_HEADER_SIZE;
	}

	current->config->process_reverse_delay = UBWT_CONFIG_PROCESS_REVERSE_DELAY_SEC;

	current->config->talk_handshake_interval = UBWT_CONFIG_TALK_HANDSHAKE_INTERVAL_MSEC;
	current->config->talk_handshake_iter = UBWT_CONFIG_TALK_HANDSHAKE_ITER;

	current->config->talk_count_default = UBWT_CONFIG_TALK_COUNT_DEFAULT;
	current->config->talk_count_current = UBWT_CONFIG_TALK_COUNT_DEFAULT;
	current->config->talk_count_max = UBWT_CONFIG_TALK_COUNT_MAX;
	current->config->talk_count_mul = 0;

	current->config->talk_payload_default_size = UBWT_CONFIG_TALK_PAYLOAD_DEFAULT_SIZE;
	current->config->talk_payload_current_size = UBWT_CONFIG_TALK_PAYLOAD_DEFAULT_SIZE;
	current->config->talk_payload_max_size = UBWT_CONFIG_TALK_PAYLOAD_MAX_SIZE;

	current->config->talk_stream_minimum_time = UBWT_CONFIG_TALK_STREAM_MINIMUM_TIME;

#ifdef UBWT_CONFIG_MULTI_THREADED
	current->config->worker_count = UBWT_CONFIG_PROCESS_WORKER_COUNT;
#endif

	_config_cmdopt_process(argc, argv);

#ifdef UBWT_CONFIG_MULTI_THREADED
	if (current->config->asynchronous) {
		/* Number of workers need to be an even number for fair asynchronous testing */
		if (current->config->worker_count & 1)
			current->config->worker_count ++;

		current->config->worker_straight_first_count = current->config->worker_count / 2;
		current->config->worker_reverse_first_count = current->config->worker_count / 2;
	} else if (current->config->reverse_first) {
		current->config->worker_straight_first_count = 0;
		current->config->worker_reverse_first_count = current->config->worker_count;
	} else {
		current->config->worker_straight_first_count = current->config->worker_count;
		current->config->worker_reverse_first_count = 0;
	}
#endif

	_config_consistency();

	_config_sanity();

	debug_info_config_show();
}

void config_destroy(void) {
	stage_set(UBWT_STAGE_STATE_DESTROY_CONFIG, 0);

	if (current->config->report_json_file) {
		memset(current->config->report_json_file, 0, strlen(current->config->report_json_file) + 1);
		free(current->config->report_json_file);
		current->config->report_json_file = NULL;
	}

	if (current->config->pid_file) {
		memset(current->config->pid_file, 0, strlen(current->config->pid_file) + 1);
		free(current->config->pid_file);
		current->config->pid_file = NULL;
	}

	if (current->config->debug_file) {
		memset(current->config->debug_file, 0, strlen(current->config->debug_file) + 1);
		free(current->config->debug_file);
		current->config->debug_file = NULL;
	}

#if 0 /* Do not wipe the entire configuration memory as it may still be required... let current_destroy() deal with it */
	memset(&current->config, 0, sizeof(current->config));
#endif
}

