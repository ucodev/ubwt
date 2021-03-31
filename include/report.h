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

#ifndef UBWT_REPORT_H
#define UBWT_REPORT_H

#include <stdint.h>

#ifndef UBWT_NO_PRAGMA_PACK
 #pragma pack(push)
 #pragma pack(4)
#endif
typedef struct
#ifdef UBWT_NO_PRAGMA_PACK
__attribute__ ((packed, aligned(4)))
#endif
ubwt_report {
	uint32_t talk_latency_us;
	uint32_t talk_count;
	uint32_t talk_stream_time;
	uint32_t talk_stream_recv_pkts;
	uint32_t __reserved1;
	uint32_t __reserved2;
	uint64_t talk_stream_recv_bytes;
} ubwt_report_t;
#ifndef UBWT_NO_PRAGMA_PACK
 #pragma pack(pop)
#endif

void report_talk_latency_set(uint32_t dt);
uint32_t report_talk_latency_get(void);
void report_talk_latency_show(void);
void report_talk_count_set(uint32_t count);
uint32_t report_talk_count_get(void);
void report_talk_count_show(void);
void report_talk_stream_time_set(uint64_t t);
uint64_t report_talk_stream_time_get(void);
void report_talk_stream_recv_pkts_set(uint64_t recv_pkts);
uint64_t report_talk_stream_recv_pkts_get(void);
void report_talk_stream_recv_bytes_set(uint64_t recv_bytes);
uint64_t report_talk_stream_recv_bytes_get(void);
void report_net_sender_connection_show(void);
void report_net_receiver_connection_show(void);
void report_marshal(void *storage, size_t len);
void report_unmarshal(const void *storage, size_t len);
void report_results_show(void);

#endif

