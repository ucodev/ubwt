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

#include "bitops.h"
#include "current.h"
#include "debug.h"
#include "stage.h"

static const char *__stage_state_str[] = {
	"UBWT_STAGE_STATE_NONE",
	"UBWT_STAGE_STATE_INIT_CURRENT",
	"UBWT_STAGE_STATE_INIT_CONFIG",
	"UBWT_STAGE_STATE_INIT_NET",
	"UBWT_STAGE_STATE_INIT_PROCESS",
	"UBWT_STAGE_STATE_RUNTIME_TALK_HANDSHAKE",
	"UBWT_STAGE_STATE_RUNTIME_TALK_NEGOTIATION",
	"UBWT_STAGE_STATE_RUNTIME_TALK_STREAM",
	"UBWT_STAGE_STATE_RUNTIME_TALK_REPORT_EXCHANGE",
	"UBWT_STAGE_STATE_RUNTIME_REPORT_SHOW",
	"UBWT_STAGE_STATE_DESTROY_PROCESS",
	"UBWT_STAGE_STATE_DESTROY_NET",
	"UBWT_STAGE_STATE_DESTROY_CONFIG",
	"UBWT_STAGE_STATE_DESTROY_CURRENT"
};

const char *stage_state_to_str(ubwt_stage_state_t state) {
	return __stage_state_str[state];
}

void stage_set(ubwt_stage_state_t state, uint32_t flags) {
	if (!bit_test(&flags, UBWT_STAGE_CTRL_FL_NO_RESET))
		stage_reset();

	current->stage.state = state;

	if (!bit_test(&flags, UBWT_STAGE_CTRL_FL_NO_SHOW))
		debug_info_stage_show();
}

ubwt_stage_state_t stage_get(void) {
	return current->stage.state;
}

void stage_inc(uint32_t flags) {
	current->stage.iter ++;

	if (!bit_test(&flags, UBWT_STAGE_CTRL_FL_NO_SHOW))
		debug_info_stage_show();
}

void stage_reset(void) {
	current->stage.state = UBWT_STAGE_STATE_NONE;
	current->stage.iter = 0;
}

