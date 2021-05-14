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

#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "bitops.h"
#include "config.h"
#include "current.h"
#include "conn.h"
#include "net.h"


int conn_has_connect(const ubwt_conn_payload_t *p) {
	return bit_test(&p->flags, UBWT_CONN_BASE_FLAG_CONNECT) && (p->magic == UBWT_CONN_MAGIC);
}

int conn_has_accept(const ubwt_conn_payload_t *p) {
	return bit_test(&p->flags, UBWT_CONN_BASE_FLAG_ACCEPT) && (p->magic == UBWT_CONN_MAGIC);
}

int conn_config_match(const ubwt_conn_payload_t *l, const ubwt_conn_payload_t *r) {
	return !memcmp(((uint8_t *) l) + offsetof(ubwt_conn_payload_t, config_flags), ((uint8_t *) r) + offsetof(ubwt_conn_payload_t, config_flags), sizeof(ubwt_conn_payload_t) - offsetof(ubwt_conn_payload_t, config_flags));
}

void conn_payload_hton(ubwt_conn_payload_t *p) {
	p->flags = htonl(p->flags);
	p->magic = htonl(p->magic);

	p->config_flags = htonl(p->config_flags);

	p->config_worker_count = htons(p->config_worker_count);
	p->config_link_mtu = htons(p->config_link_mtu);
	p->config_l4_payload_size = htons(p->config_l4_payload_size);
	p->config_conn_timeout = htons(p->config_conn_timeout);
	p->config_handshake_iter = htons(p->config_handshake_iter);
	p->config_handshake_interval = htons(p->config_handshake_interval);
	p->config_stream_run_timeout = htons(p->config_stream_run_timeout);
	p->config_stream_time = htonl(p->config_stream_time);
	p->config_count_mul = htonl(p->config_count_mul);
}

void conn_payload_ntoh(ubwt_conn_payload_t *p) {
	p->flags = ntohl(p->flags);
	p->magic = ntohl(p->magic);

	p->config_flags = ntohl(p->config_flags);

	p->config_worker_count = ntohs(p->config_worker_count);
	p->config_link_mtu = ntohs(p->config_link_mtu);
	p->config_l4_payload_size = ntohs(p->config_l4_payload_size);
	p->config_conn_timeout = ntohs(p->config_conn_timeout);
	p->config_handshake_iter = ntohs(p->config_handshake_iter);
	p->config_handshake_interval = ntohs(p->config_handshake_interval);
	p->config_stream_run_timeout = ntohs(p->config_stream_run_timeout);
	p->config_stream_time = ntohl(p->config_stream_time);
	p->config_count_mul = ntohl(p->config_count_mul);
}

void conn_payload_create(ubwt_conn_payload_t *p) {
	memset(p, 0, sizeof(ubwt_conn_payload_t));

	if (net_im_connector()) {
		bit_set(&p->flags, UBWT_CONN_BASE_FLAG_CONNECT);
	} else {
		bit_set(&p->flags, UBWT_CONN_BASE_FLAG_ACCEPT);
	}

	p->magic = UBWT_CONN_MAGIC;

	if (current->config->asynchronous)
		bit_set(&p->config_flags, UBWT_CONN_CONFIG_FLAG_ASYNC);
	
	if (current->config->bidirectional)
		bit_set(&p->config_flags, UBWT_CONN_CONFIG_FLAG_BIDIR);
	
	if (current->config->reverse_first)
		bit_set(&p->config_flags, UBWT_CONN_CONFIG_FLAG_REVRS);

	bit_set(&p->config_flags, UBWT_CONN_CONFIG_FLAG_MTU);
	p->config_link_mtu = current->config->net_mtu;

	bit_set(&p->config_flags, UBWT_CONN_CONFIG_FLAG_HSINTERV);
	p->config_handshake_interval = current->config->talk_handshake_interval;

	bit_set(&p->config_flags, UBWT_CONN_CONFIG_FLAG_HSITER);
	p->config_handshake_iter = current->config->talk_handshake_iter;

	bit_set(&p->config_flags, UBWT_CONN_CONFIG_FLAG_CTIMEO);
	p->config_conn_timeout = current->config->net_timeout_default;

	bit_set(&p->config_flags, UBWT_CONN_CONFIG_FLAG_STIMEO);
	p->config_stream_run_timeout = current->config->net_timeout_talk_stream_run;

	bit_set(&p->config_flags, UBWT_CONN_CONFIG_FLAG_STIME);
	p->config_stream_time = current->config->talk_stream_minimum_time;

	bit_set(&p->config_flags, UBWT_CONN_CONFIG_FLAG_CNTMUL);
	p->config_count_mul = current->config->talk_count_mul;

	bit_set(&p->config_flags, UBWT_CONN_CONFIG_FLAG_PLSIZE);
	p->config_l4_payload_size = current->config->talk_payload_current_size;

	bit_set(&p->config_flags, UBWT_CONN_CONFIG_FLAG_WRKCNT);
#ifdef UBWT_CONFIG_MULTI_THREADED
	p->config_worker_count = current->config->worker_count;
#else
	p->config_worker_count = 1;
#endif
}

