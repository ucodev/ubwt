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

#include <string.h>

#include <errno.h>

#include "current.h"
#include "datetime.h"
#include "stage.h"

static struct ubwt_current __current;
struct ubwt_current *current = &__current;

void current_init(void) {
	memset(&__current, 0, sizeof(struct ubwt_current));

	stage_set(UBWT_STAGE_STATE_INIT_CURRENT, 0);

	current->time_us = datetime_now_us();
}

void current_update(void) {
	current->time_us = datetime_now_us();
}

