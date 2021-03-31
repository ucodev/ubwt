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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include <sys/time.h>

#include "error.h"
#include "datetime.h"

uint64_t datetime_now_us(void) {
	struct timeval tv = { 0, 0 };
	uint64_t t_us = 0;

	if (gettimeofday(&tv, NULL) < 0) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_TIME_GET, "time_now_us(): gettimeofday()");
		error_no_return();
	}

	t_us = (uint64_t) (tv.tv_sec * 1000000) + (unsigned long) tv.tv_usec;

	return t_us;
}

char *datetime_now_str(char *buf) {
	time_t t = time(NULL);

	if (t == (time_t) -1) {
		buf[0] = 0;
		return NULL;
	}

	if (!ctime_r(&t, buf)) {
		buf[0] = 0;
		return NULL;
	}

	buf[strlen(buf) - 1] = 0;

	return buf;
}

