/*
 
    ubwt - uCodev Bandwidth Tester
    Copyright (C) 2021  Pedro A. Hortas <pah@ucodev.org>

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

static void _config_sanity(void) {
	assert(strlen(current->config.port) <= 5 && atoi(current->config.port) >= 1 && (atoi(current->config.port) <= 65535));

	assert(current->config.net_mtu >= UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE);
	assert(current->config.net_timeout_default > 0);
	assert(current->config.net_l4_proto_value != 0);

	assert(current->config.talk_handshake_iter > 0);

	assert(current->config.talk_count_current > 0);
	assert(current->config.talk_count_default > 0);
	assert(current->config.talk_count_max > 0);

	assert(current->config.talk_payload_current_size >= UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE);
	assert(current->config.talk_payload_default_size >= UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE);
	assert(current->config.talk_payload_max_size >= UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE);

	assert(current->config.talk_payload_current_size > offsetof(ubwt_talk_payload_t, buf));
	assert(current->config.talk_payload_default_size > offsetof(ubwt_talk_payload_t, buf));
	assert(current->config.talk_payload_max_size > offsetof(ubwt_talk_payload_t, buf));
	assert(current->config.talk_payload_max_size >= current->config.talk_payload_current_size);
	assert(current->config.talk_payload_max_size >= current->config.talk_payload_default_size);

	assert(current->config.talk_stream_minimum_time > 0);
}

static void _config_cmdopt_process(int argc, char *const *argv) {
	int opt = 0;
	const char *flags =
#ifdef UBWT_CONFIG_DEBUG
		"d"
#endif
		"hl:m:p:s:t:w:";

	while ((opt = getopt(argc, argv, flags)) != -1) {
		switch (opt) {
#ifdef UBWT_CONFIG_DEBUG
			case 'd': {
				current->config.debug = 1;
			} break;
#endif

			case 'h': {
				usage_show(argv, 1);
				error_no_return();
			} break;

			case 'l': {
				assert(strlen(optarg) < sizeof(current->config.net_l4_proto_name));
				strncpy(current->config.net_l4_proto_name, optarg, sizeof(current->config.net_l4_proto_name) - 1);
				current->config.net_l4_proto_name[sizeof(current->config.net_l4_proto_name) - 1] = 0;

				if (!strcmp(optarg, "tcp")) {
					current->config.net_l4_proto_value = UBWT_NET_PROTO_L4_TCP;
				} else if (!strcmp(optarg, "udp")) {
					current->config.net_l4_proto_value = UBWT_NET_PROTO_L4_UDP;
				} else {
					usage_show(argv, 0);
					error_no_return();
				}
			} break;

			case 'm': {
				assert(atoi(optarg) >= UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE && atoi(optarg) < 65536);
				current->config.net_mtu = (uint16_t) atoi(optarg);
			} break;

			case 'p': {
				strncpy(current->config.port, optarg, sizeof(current->config.port) - 1);
				current->config.port[sizeof(current->config.port) - 1] = 0;
			} break;

			case 's': {
				assert(atoi(optarg) >= UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE && atoi(optarg) < 65536);
				current->config.talk_payload_default_size = (uint16_t) atoi(optarg);
			} break;

			case 't': {
				assert(atoi(optarg) > 0 && atoi(optarg) < 65536);
				current->config.talk_stream_minimum_time = (uint16_t) atoi(optarg);
			} break;

			case 'w': {
				assert(atoi(optarg) > 0 && atoi(optarg) < 65536);
				current->config.net_timeout_default = (uint16_t) atoi(optarg);
			} break;


			default: {
				errno = EINVAL;
				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPT_INVALID, "_config_cmdopt_process()");
				error_no_return();
			}
		}
	}

#ifdef UBWT_CONFIG_DEBUG
	if (current->config.debug)
		current->config.error_log_min_level = UBWT_ERROR_LEVEL_INFO;
#endif

	if ((argc - optind) != 2) {
		usage_show(argv, 0);
		error_no_return();
	}

	if (!strcmp(argv[optind], "sender")) {
		current->config.im_sender = 1;
	} else if (!strcmp(argv[optind], "receiver")) {
		current->config.im_receiver = 1;
	} else {
		errno = EINVAL;
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_1_INVALID, "config_init()");
		error_no_return();
	}

	optind ++;

	assert(strlen(argv[optind]) < sizeof(current->config.addr));

	memset(current->config.addr, 0, sizeof(current->config.addr));
	strncpy(current->config.addr, argv[optind], sizeof(current->config.addr) - 1);
	current->config.addr[sizeof(current->config.addr) - 1] = 0;
}

void config_init(int argc, char *const *argv) {
	stage_set(UBWT_STAGE_STATE_INIT_CONFIG, 0);

	memset(&current->config, 0, sizeof(current->config));

	current->config.error_log_min_level = UBWT_ERROR_LEVEL_CRITICAL;

	strncpy(current->config.port, UBWT_CONFIG_PORT_DEFAULT, sizeof(current->config.port) - 1);
	current->config.port[sizeof(current->config.port) - 1] = 0;

	current->config.net_timeout_default = UBWT_CONFIG_NET_TIMEOUT_DEFAULT;
	current->config.net_timeout_talk_stream_run = UBWT_CONFIG_NET_TIMEOUT_TALK_STREAM_RUN;
	current->config.net_timeout_talk_stream_end = UBWT_CONFIG_NET_TIMEOUT_TALK_STREAM_END;

	current->config.net_mtu = UBWT_CONFIG_NET_MTU;
	current->config.net_l2_hdr_size = UBWT_CONFIG_NET_L2_HEADER_SIZE;
	current->config.net_l3_ipv4_hdr_size = UBWT_CONFIG_NET_L3_IPV4_HEADER_SIZE;
	current->config.net_l3_ipv6_hdr_size = UBWT_CONFIG_NET_L3_IPV6_HEADER_SIZE;
	strncpy(current->config.net_l4_proto_name, UBWT_CONFIG_NET_L4_PROTO_DEFAULT, sizeof(current->config.net_l4_proto_name) - 1);
	current->config.net_l4_proto_name[sizeof(current->config.net_l4_proto_name) - 1] = 0;

	if (!strcmp(UBWT_CONFIG_NET_L4_PROTO_DEFAULT, "tcp")) {
		current->config.net_l4_proto_value = UBWT_NET_PROTO_L4_TCP;
		current->config.net_l4_hdr_size = UBWT_CONFIG_NET_L4_TCP_HEADER_SIZE;
	} else if (!strcmp(UBWT_CONFIG_NET_L4_PROTO_DEFAULT, "udp")) {
		current->config.net_l4_proto_value = UBWT_NET_PROTO_L4_UDP;
		current->config.net_l4_hdr_size = UBWT_CONFIG_NET_L4_UDP_HEADER_SIZE;
	}

	current->config.talk_handshake_iter = UBWT_CONFIG_TALK_HANDSHAKE_ITER;

	current->config.talk_count_default = UBWT_CONFIG_TALK_COUNT_DEFAULT;
	current->config.talk_count_current = UBWT_CONFIG_TALK_COUNT_DEFAULT;
	current->config.talk_count_max = UBWT_CONFIG_TALK_COUNT_MAX;

	current->config.talk_payload_default_size = UBWT_CONFIG_TALK_PAYLOAD_DEFAULT_SIZE;
	current->config.talk_payload_current_size = UBWT_CONFIG_TALK_PAYLOAD_DEFAULT_SIZE;
	current->config.talk_payload_max_size = UBWT_CONFIG_TALK_PAYLOAD_MAX_SIZE;

	current->config.talk_stream_minimum_time = UBWT_CONFIG_TALK_STREAM_MINIMUM_TIME;

	_config_cmdopt_process(argc, argv);

	_config_sanity();

	debug_info_config_show();
}

void config_destroy(void) {
	stage_set(UBWT_STAGE_STATE_DESTROY_CONFIG, 0);

#if 0 /* Do not wipe configuration memory as it may still be required... let current_destroy() deal with it */
	memset(&current->config, 0, sizeof(current->config));
#endif
}

