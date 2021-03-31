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

#include "config.h"
#include "current.h"
#include "net.h"
#include "process.h"
#include "stage.h"
#include "talk.h"

void process_init(void) {
	stage_set(UBWT_STAGE_STATE_INIT_PROCESS, 0);

	current_update();

	if (net_im_receiver()) {
		talk_receiver();
	} else {
		talk_sender();
	}

	process_report();
}

void process_report(void) {
	stage_set(UBWT_STAGE_STATE_RUNTIME_REPORT_SHOW, 0);

	report_results_show();
}

