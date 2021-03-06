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

#ifndef UBWT_REPORT_H
#define UBWT_REPORT_H

#include <stdio.h>
#include <stdint.h>

#ifndef UBWT_CONFIG_NO_PRAGMA_PACK
 #pragma pack(push)
 #pragma pack(4)
#endif
typedef struct
#ifdef UBWT_CONFIG_NO_PRAGMA_PACK
__attribute__ ((packed, aligned(4)))
#endif
ubwt_report_collect {
	/* Collected data */
	uint32_t talk_latency_us;
	uint32_t talk_count;
	uint32_t talk_stream_recv_pkts;
	uint32_t __reserved1;
	uint32_t __reserved2;
	uint32_t __reserved3;
	uint64_t talk_stream_time_duration; /* Duration of stream run (usec) */
	uint64_t talk_stream_time_start; /* Start time of stream run (UNIX time in usec) */
	uint64_t talk_stream_recv_bytes;
} ubwt_report_collect_t;
#ifndef UBWT_CONFIG_NO_PRAGMA_PACK
 #pragma pack(pop)
#endif

struct ubwt_report_compute {
	/* Computed data */
	uint32_t bandwidth_theoretical_mbps;
	double   bandwidth_estimated_mbps;
	double   bandwidth_effective_mbps;
	double   packet_loss;
	double   fragmentation_ratio;
	uint64_t total_pkts;
	size_t   hdr_size;
};

struct ubwt_report_result {
	/* Collected data */
	ubwt_report_collect_t collected;

	/* Computed data */
	struct ubwt_report_compute computed;
};

struct ubwt_report {
	FILE *fp_export_json;
	struct ubwt_report_result *result;
	struct ubwt_report_result  results[2];
};

#ifdef UBWT_CONFIG_MULTI_THREADED
void report_worker_combine(void);
#endif
void report_set_straight(void);
void report_set_reverse(void);
void report_talk_latency_set(uint32_t dt);
uint32_t report_talk_latency_get(void);
void report_talk_latency_show(void);
void report_talk_count_set(uint32_t count);
uint32_t report_talk_count_get(void);
void report_talk_count_show(void);
void report_talk_stream_time_start_set(uint64_t t);
uint64_t report_talk_stream_time_start_get(void);
void report_talk_stream_time_duration_set(uint64_t t);
uint64_t report_talk_stream_time_duration_get(void);
void report_talk_stream_recv_pkts_set(uint32_t recv_pkts);
uint32_t report_talk_stream_recv_pkts_get(void);
void report_talk_stream_recv_bytes_set(uint64_t recv_bytes);
uint64_t report_talk_stream_recv_bytes_get(void);
void report_net_connector_connection_show(void);
void report_net_listener_connection_show(void);
void report_marshal(void *storage, size_t len);
void report_unmarshal(const void *storage, size_t len);
void report_results_compute(void);
void report_results_show(void);
void report_export_json_start(void);
void report_export_json_dump(int resume);
void report_export_json_end(void);
void report_init(void);
void report_destroy(void);

#endif

