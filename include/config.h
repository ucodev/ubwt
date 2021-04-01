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

#ifndef UBWT_CONFIG_H
#define UBWT_CONFIG_H

#include <stdint.h>

#include "error.h"
#include "net.h"

#define UBWT_CONFIG_DEBUG				1
#define UBWT_CONFIG_PORT_DEFAULT			"19991"
#define UBWT_CONFIG_NET_TIMEOUT_DEFAULT			120
#define UBWT_CONFIG_NET_TIMEOUT_TALK_STREAM_RUN		1
#define UBWT_CONFIG_NET_TIMEOUT_TALK_STREAM_END		20
#define UBWT_CONFIG_NET_MTU				1500
#define UBWT_CONFIG_NET_L2_HEADER_SIZE			26		/* Ethernet (no trailers) */
#define UBWT_CONFIG_NET_L3_IPV4_HEADER_SIZE		20		/* IPv4 (minimum) */
#define UBWT_CONFIG_NET_L3_IPV6_HEADER_SIZE		40		/* IPv6 (minimum) */
#define UBWT_CONFIG_NET_L4_HEADER_SIZE			8		/* UDP */
#define UBWT_CONFIG_NET_L4_PROTO			"udp"
#define UBWT_CONFIG_TALK_HANDSHAKE_ITER			20
#define UBWT_CONFIG_TALK_COUNT_DEFAULT			100
#define UBWT_CONFIG_TALK_COUNT_MAX			100000000
#define UBWT_CONFIG_TALK_PAYLOAD_DEFAULT_SIZE		1432
#define UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE		508
#define UBWT_CONFIG_TALK_PAYLOAD_MAX_SIZE		8192
#define UBWT_CONFIG_TALK_STREAM_MINIMUM_TIME		2

#if defined(UBWT_CONFIG_DEBUG) && (UBWT_CONFIG_DEBUG == 0)
 #undef UBWT_CONFIG_DEBUG
#endif

//#define UBWT_NO_PRAGMA_PACK 1

#ifndef UBWT_NO_PRAGMA_PACK
 #pragma pack(push)
 #pragma pack(4)
#endif
struct
#ifdef UBWT_NO_PRAGMA_PACK
__attribute__ ((packed, aligned(4)))
#endif
ubwt_config {
	unsigned int im_sender;
	unsigned int im_receiver;

	char addr[1024];
	char port[8];

	uint8_t debug;

	ubwt_error_level_t error_log_min_level;

	uint16_t net_timeout_default;
	uint16_t net_timeout_talk_stream_run;
	uint16_t net_timeout_talk_stream_end;
	uint16_t net_mtu;
	uint16_t net_l2_hdr_size;
	uint16_t net_l3_ipv4_hdr_size;
	uint16_t net_l3_ipv6_hdr_size;
	uint16_t net_l4_hdr_size;
	char     net_l4_proto[16];

	uint16_t talk_handshake_iter;
	uint32_t talk_count_current;
	uint32_t talk_count_default;
	uint32_t talk_count_max;
	uint16_t talk_payload_current_size;
	uint16_t talk_payload_default_size;
	uint16_t talk_payload_max_size;
	uint16_t talk_stream_minimum_time;
};
#ifndef UBWT_NO_PRAGMA_PACK
 #pragma pack(pop)
#endif

void config_init(int argc, char *const *argv);

#endif

