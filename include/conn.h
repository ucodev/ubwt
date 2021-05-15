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

#ifndef UBWT_CONN_H
#define UBWT_CONN_H

#include <stdint.h>

#define UBWT_CONN_MAGIC		0x47A8F3CD

typedef enum UBWT_CONN_BASE_FLAGS {
	UBWT_CONN_BASE_FLAG_CONNECT = 1,
	UBWT_CONN_BASE_FLAG_ACCEPT
} ubwt_conn_base_flag_t;

typedef enum UBWT_CONN_CONFIG_FLAGS {
	UBWT_CONN_CONFIG_FLAG_ASYNC = 1,
	UBWT_CONN_CONFIG_FLAG_BIDIR,
	UBWT_CONN_CONFIG_FLAG_REVRS,
	UBWT_CONN_CONFIG_FLAG_MTU,
	UBWT_CONN_CONFIG_FLAG_HSINTERV,
	UBWT_CONN_CONFIG_FLAG_HSITER,
	UBWT_CONN_CONFIG_FLAG_CTIMEO,
	UBWT_CONN_CONFIG_FLAG_STIME,
	UBWT_CONN_CONFIG_FLAG_PLSIZE,
	UBWT_CONN_CONFIG_FLAG_WRKCNT,
	UBWT_CONN_CONFIG_FLAG_CNTMUL,
	UBWT_CONN_CONFIG_FLAG_STIMEO,
	UBWT_CONN_CONFIG_FLAG_REVDLY
} ubwt_conn_config_flag_t;

#ifndef UBWT_CONFIG_NO_PRAGMA_PACK
 #pragma pack(push)
 #pragma pack(4)
#endif
typedef struct
#ifdef UBWT_CONFIG_NO_PRAGMA_PACK
__attribute__ ((packed, aligned(4)))
#endif
ubwt_conn_payload {
	uint32_t flags;
	uint32_t magic;

	uint32_t config_flags;

	uint16_t config_worker_count;
	uint16_t config_link_mtu;
	uint16_t config_l4_payload_size;
	uint16_t config_conn_timeout;
	uint16_t config_handshake_iter;
	uint16_t config_handshake_interval;
	uint16_t config_stream_run_timeout;
	uint16_t config_reverse_delay;
	uint32_t config_stream_time;
	uint32_t config_count_mul;
} ubwt_conn_payload_t;
#ifndef UBWT_CONFIG_NO_PRAGMA_PACK
 #pragma pack(pop)
#endif

int conn_has_connect(const ubwt_conn_payload_t *p);
int conn_has_accept(const ubwt_conn_payload_t *p);
int conn_config_match(const ubwt_conn_payload_t *l, const ubwt_conn_payload_t *r);
void conn_payload_hton(ubwt_conn_payload_t *p);
void conn_payload_ntoh(ubwt_conn_payload_t *p);
void conn_payload_create(ubwt_conn_payload_t *p);

#endif

