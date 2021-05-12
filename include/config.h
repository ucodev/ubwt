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

#ifndef UBWT_CONFIG_H
#define UBWT_CONFIG_H

#include <stdint.h>

#include "error.h"
#include "net.h"

#define UBWT_CONFIG_VERSION_STR				"0.1-alpha-03"
#define UBWT_CONFIG_CTIME_SIZE				32
#define UBWT_CONFIG_PORT_DEFAULT			"19991"
#define UBWT_CONFIG_NET_TIMEOUT_DEFAULT			120
#define UBWT_CONFIG_NET_TIMEOUT_TALK_STREAM_RUN		5
#define UBWT_CONFIG_NET_TIMEOUT_TALK_STREAM_END		20
#define UBWT_CONFIG_NET_MTU				1500
#define UBWT_CONFIG_NET_LISTEN_BACKLOG			10
#define UBWT_CONFIG_NET_L2_HEADER_SIZE			26		/* Ethernet (no trailers) */
#define UBWT_CONFIG_NET_L3_IPV4_HEADER_SIZE		20		/* IPv4 (minimum) */
#define UBWT_CONFIG_NET_L3_IPV6_HEADER_SIZE		40		/* IPv6 (minimum) */
#define UBWT_CONFIG_NET_L4_UDP_HEADER_SIZE		8		/* UDP (fixed) */
#define UBWT_CONFIG_NET_L4_TCP_HEADER_SIZE		20		/* TCP (minimum) */
#define UBWT_CONFIG_NET_L4_PROTO_DEFAULT		"tcp"
#define UBWT_CONFIG_PROCESS_WORKER_COUNT		1
#define UBWT_CONFIG_PROCESS_REVERSE_DELAY_SEC		2
#define UBWT_CONFIG_TALK_PROTO_VER			1
#define UBWT_CONFIG_TALK_HANDSHAKE_INTERVAL_MSEC	1
#define UBWT_CONFIG_TALK_HANDSHAKE_ITER			20
#define UBWT_CONFIG_TALK_COUNT_DEFAULT			100
#define UBWT_CONFIG_TALK_COUNT_MAX			1000000000
#define UBWT_CONFIG_TALK_PAYLOAD_DEFAULT_SIZE		1432
#define UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE		508
#define UBWT_CONFIG_TALK_PAYLOAD_MAX_SIZE		8192
#define UBWT_CONFIG_TALK_STREAM_MINIMUM_TIME		3

/* uConf */
//#define UBWT_CONFIG_DEBUG				1
//#define UBWT_CONFIG_NO_PRAGMA_PACK			1
//#define UBWT_CONFIG_NET_NO_UDP			0
//#define UBWT_CONFIG_NET_REUSE_ADDRESS			1
//#define UBWT_CONFIG_NET_REUSE_PORT			1
//#define UBWT_CONFIG_NET_USE_SETSOCKOPT		1
//#define UBWT_CONFIG_NET_UDP_CONNECT			1
//#define UBWT_CONFIG_MULTI_THREADED			1
//#define UBWT_CONFIG_CUSTOM_PTHREAD_BARRIER		0
//#define COMPILE_WIN32					0

#if defined(UBWT_CONFIG_DEBUG) && (UBWT_CONFIG_DEBUG == 0)
 #undef UBWT_CONFIG_DEBUG
#endif

#if defined(UBWT_CONFIG_NO_PRAGMA_PACK) && (UBWT_CONFIG_NO_PRAGMA_PACK == 0)
 #undef UBWT_CONFIG_NO_PRAGMA_PACK
#endif

#if defined(UBWT_CONFIG_NET_NO_UDP) && (UBWT_CONFIG_NET_NO_UDP == 0)
 #undef UBWT_CONFIG_NET_NO_UDP
#endif

#if defined(UBWT_CONFIG_NET_REUSE_ADDRESS) && (UBWT_CONFIG_NET_REUSE_ADDRESS == 0)
 #undef UBWT_CONFIG_NET_REUSE_ADDRESS
#endif

#if defined(UBWT_CONFIG_NET_REUSE_PORT) && (UBWT_CONFIG_NET_REUSE_PORT == 0)
 #undef UBWT_CONFIG_NET_REUSE_PORT
#endif

#if defined(UBWT_CONFIG_NET_USE_SETSOCKOPT) && (UBWT_CONFIG_NET_USE_SETSOCKOPT == 0)
 #undef UBWT_CONFIG_NET_USE_SETSOCKOPT
#endif

#if defined(UBWT_CONFIG_NET_UDP_CONNECT) && (UBWT_CONFIG_NET_UDP_CONNECT == 0)
 #undef UBWT_CONFIG_NET_UDP_CONNECT
#endif

#if defined(UBWT_CONFIG_MULTI_THREADED) && (UBWT_CONFIG_MULTI_THREADED == 0)
 #undef UBWT_CONFIG_MULTI_THREADED
#endif

#if defined(UBWT_CONFIG_CUSTOM_PTHREAD_BARRIER) && (UBWT_CONFIG_CUSTOM_PTHREAD_BARRIER == 0)
 #undef UBWT_CONFIG_CUSTOM_PTHREAD_BARRIER
#endif

#if defined(COMPILE_WIN32) && (COMPILE_WIN32 == 0)
 #undef COMPILE_WIN32
#endif

#if defined(UBWT_CONFIG_NET_NO_UDP) && defined(UBWT_CONFIG_NET_UDP_CONNECT)
 #undef UBWT_CONFIG_NET_UDP_CONNECT
#endif

/* !uConf */

struct ubwt_config {
	unsigned int im_connector;
	unsigned int im_listener;

	char addr[1024];
	char port[8];

	uint8_t debug;
	uint8_t report_full;
	uint8_t bidirectional;
	uint8_t asynchronous;
	uint8_t reverse_first;

	char *report_json_file;
	char *pid_file;
	char *debug_file;

	ubwt_error_level_t error_log_min_level;

	uint16_t net_timeout_default;
	uint16_t net_timeout_talk_stream_run;
	uint16_t net_timeout_talk_stream_end;
	uint16_t net_mtu;
	int      net_reuseaddr;
	int      net_reuseport;
	uint16_t net_l2_hdr_size;
	uint16_t net_l3_ipv4_hdr_size;
	uint16_t net_l3_ipv6_hdr_size;
	uint16_t net_l4_hdr_size;
	char     net_l4_proto_name[16];
	uint8_t  net_l4_proto_value;

	uint16_t process_reverse_delay;

	uint16_t talk_handshake_interval;
	uint16_t talk_handshake_iter;
	uint32_t talk_count_current;
	uint32_t talk_count_default;
	uint32_t talk_count_max;
	uint16_t talk_payload_current_size;
	uint16_t talk_payload_default_size;
	uint16_t talk_payload_max_size;
	uint16_t talk_stream_minimum_time;

#ifdef UBWT_CONFIG_MULTI_THREADED
	unsigned int worker_count;
	unsigned int worker_straight_first_count, worker_reverse_first_count;
#endif
};

extern struct ubwt_config __config;

void config_init(int argc, char *const *argv);
void config_destroy(void);

#endif

