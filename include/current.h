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

#ifndef UBWT_CURRENT_H
#define UBWT_CURRENT_H

#include <stdint.h>

#include "config.h"
#include "error.h"
#include "net.h"
#include "report.h"
#include "stage.h"
#include "process.h"

struct ubwt_current {
	struct ubwt_config config;
	struct ubwt_net net;
	uint64_t time_us;

	ubwt_stage_t stage;
	ubwt_error_t error;

	struct ubwt_report report;
	struct ubwt_process process;
};

extern struct ubwt_current *current;

void current_init(void);
void current_update(void);
void current_destroy(void);

#endif

