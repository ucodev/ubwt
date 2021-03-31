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

#ifndef UBWT_STAGE_H
#define UBWT_STAGE_H

enum UBWT_STAGE_CONTROL_FLAGS {
	UBWT_STAGE_CTRL_FL_NO_RESET = 1,
	UBWT_STAGE_CTRL_FL_NO_SHOW
};

typedef enum UBWT_STAGE_STATES {
	UBWT_STAGE_STATE_NONE = 0,
	UBWT_STAGE_STATE_INIT_CURRENT,
	UBWT_STAGE_STATE_INIT_CONFIG,
	UBWT_STAGE_STATE_INIT_NET,
	UBWT_STAGE_STATE_INIT_PROCESS,
	UBWT_STAGE_STATE_RUNTIME_TALK_HANDSHAKE,
	UBWT_STAGE_STATE_RUNTIME_TALK_NEGOTIATION,
	UBWT_STAGE_STATE_RUNTIME_TALK_STREAM,
	UBWT_STAGE_STATE_RUNTIME_TALK_REPORT_EXCHANGE,
	UBWT_STAGE_STATE_RUNTIME_REPORT_SHOW
} ubwt_stage_state_t;

typedef struct ubwt_stage {
	size_t iter;
	ubwt_stage_state_t state;
} ubwt_stage_t;

const char *stage_state_to_str(ubwt_stage_state_t state);
void stage_set(ubwt_stage_state_t stage, uint32_t flags);
ubwt_stage_state_t stage_get(void);
void stage_inc(uint32_t flags);
void stage_reset(void);

#endif

