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

void process_set_im_sender(void) {
	current->process.im_sender = 1;
	current->process.im_receiver = 0;
}

void process_set_im_receiver(void) {
	current->process.im_sender = 0;
	current->process.im_receiver = 1;
}

void process_init(void) {
	stage_set(UBWT_STAGE_STATE_INIT_PROCESS, 0);

	current_update();
}

void process_run(void) {
	if (net_im_listener()) {
		report_set_straight();

		process_set_im_receiver();

		talk_init();

		talk_receiver();

		if (!current->config.bidirectional) {
			talk_destroy();
			return ;
		}


		/* Reverse testing */

		report_set_reverse();

		process_set_im_sender();

		talk_sender();

		talk_destroy();
	} else {
		report_set_straight();

		process_set_im_sender();

		talk_init();

		talk_sender();

		if (!current->config.bidirectional) {
			talk_destroy();
			return ;
		}


		/* Reverse testing */

		report_set_reverse();

		process_set_im_receiver();

		talk_receiver();

		talk_destroy();
	}
}

void process_report(void) {
	stage_set(UBWT_STAGE_STATE_RUNTIME_REPORT_SHOW, 0);

	if (net_im_listener()) {
		process_set_im_receiver();
	} else {
		process_set_im_sender();
	}

	report_set_straight();
	report_results_compute();
	report_results_show();

	if (!current->config.bidirectional)
		return;


	/* Reverse testing results */

	if (net_im_listener()) {
		process_set_im_sender();
	} else {
		process_set_im_receiver();
	}

	report_set_reverse();
	report_results_compute();
	report_results_show();
}

void process_destroy(void) {
	stage_set(UBWT_STAGE_STATE_DESTROY_PROCESS, 0);

	current_update();
}

