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

#include <unistd.h>

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

void process_set_reverse(void) {
	current->process.reverse = 1;
}

void process_set_straight(void) {
	current->process.reverse = 0;
}

void process_init(void) {
	stage_set(UBWT_STAGE_STATE_INIT_PROCESS, 0);

	current_update();
}

static void _process_listener_straight(int init, int cleanup) {
	process_set_straight();

	report_set_straight();

	process_set_im_receiver();

	if (init)
		talk_init();

	talk_receiver();

	if (cleanup)
		talk_destroy();
}

static void _process_connector_straight(int init, int cleanup) {
	process_set_straight();

	report_set_straight();

	process_set_im_sender();

	if (init)
		talk_init();

	talk_sender();

	if (cleanup)
		talk_destroy();
}

static void _process_listener_reverse(int init, int cleanup) {
	process_set_reverse();

	report_set_reverse();

	process_set_im_sender();

	if (init)
		talk_init();

	talk_sender();

	if (cleanup)
		talk_destroy();
}

static void _process_connector_reverse(int init, int cleanup) {
	process_set_reverse();

	report_set_reverse();

	process_set_im_receiver();

	if (init)
		talk_init();

	talk_receiver();

	if (cleanup)
		talk_destroy();
}

static void _process_run_straight_first(void) {
	if (net_im_listener()) {
		if (current->config->bidirectional) {
			_process_listener_straight(1 /* init */, 0);
		} else {
			_process_listener_straight(1 /* init */, 1 /* cleanup */);
			return ;
		}

		/* Wait a bit before transmitting */

		sleep(current->config->process_reverse_delay);

		/* Reverse testing */

		_process_listener_reverse(0, 1 /* cleanup */);
	} else {
		if (current->config->bidirectional) {
			_process_connector_straight(1 /* init */, 0);
		} else {
			_process_connector_straight(1 /* init */, 1 /* cleanup */);
			return ;
		}

		/* Reverse testing */

		_process_connector_reverse(0, 1 /* cleanup */);
	}
}

static void _process_run_reverse_first(void) {
	if (net_im_listener()) {
		if (current->config->bidirectional) {
			_process_listener_reverse(1 /* init */, 0);
		} else {
			_process_listener_reverse(1 /* init */, 1 /* cleanup */);
			return ;
		}

		/* Wait a bit before transmitting */

		sleep(current->config->process_reverse_delay);

		/* Straight testing */

		_process_listener_straight(0, 1 /* cleanup */);
	} else {
		if (current->config->bidirectional) {
			_process_connector_reverse(1 /* init */, 0);
		} else {
			_process_connector_reverse(1 /* init */, 1 /* cleanup */);
			return ;
		}

		/* Straight testing */

		_process_connector_straight(0, 1 /* cleanup */);
	}
}

void process_run(int reverse_first) {
	if (reverse_first) {
		_process_run_reverse_first();
	} else {
		_process_run_straight_first();
	}

#ifdef UBWT_CONFIG_MULTI_THREADED
	worker_task_exit();
#endif
}

void process_report(void) {
	stage_set(UBWT_STAGE_STATE_RUNTIME_REPORT_SHOW, 0);

#ifdef UBWT_CONFIG_MULTI_THREADED
	report_worker_combine();
#endif

	if (net_im_listener()) {
		process_set_im_receiver();
	} else {
		process_set_im_sender();
	}

	report_set_straight();
	report_results_compute();
	report_results_show();

	if (!current->config->bidirectional)
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

