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

#ifndef UBWT_TALK_H
#define UBWT_TALK_H

#include <stdint.h>

typedef enum UBWT_TALK_OPS {
	UBWT_TALK_OP_NOOP = 0,
	UBWT_TALK_OP_PING,
	UBWT_TALK_OP_PONG,
	UBWT_TALK_OP_LATENCY_US,
	UBWT_TALK_OP_COUNT_REQ,
	UBWT_TALK_OP_STREAM_START,
	UBWT_TALK_OP_STREAM_RUN,
	UBWT_TALK_OP_STREAM_END,
	UBWT_TALK_OP_STREAM_WEAK,
	UBWT_TALK_OP_REPORT,
	UBWT_TALK_OP_FORCE_FAIL
} ubwt_talk_ops_t;

#ifndef UBWT_NO_PRAGMA_PACK
 #pragma pack(push)
 #pragma pack(4)
#endif
typedef struct
#ifdef UBWT_NO_PRAGMA_PACK
__attribute__ ((packed, aligned(4)))
#endif
ubwt_talk_payload {
	uint32_t flags;
	uint32_t count;
	uint32_t latency;
	uint32_t __reserved;
	uint8_t buf[UBWT_CONFIG_TALK_PAYLOAD_MAX_SIZE - 16];
} ubwt_talk_payload_t;
#ifndef UBWT_NO_PRAGMA_PACK
 #pragma pack(pop)
#endif

const char *talk_op_to_str(ubwt_talk_ops_t op);
void talk_init(void);
void talk_destroy(void);
void talk_sender(void);
void talk_receiver(void);

#endif

