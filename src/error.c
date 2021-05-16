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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#include <sys/types.h>

#if !defined(__GNUC__) && !defined(__clang__)
 #include <stdnoreturn.h>
#endif

#include "current.h"
#include "datetime.h"
#include "error.h"
#include "net.h"

static const char *__errors[] = {
	"[%s]: ERROR: %s: No error: %s: %s\n",
	"[%s]: ERROR: %s: Unable to fork the current process data: %s: %s\n",
	"[%s]: ERROR: %s: First parameter must be 'connector' or 'listener': %s: %s\n",
	"[%s]: ERROR: %s: Second parameter must be a valid IP or hostname: %s: %s\n",
	"[%s]: ERROR: %s: Invalid command line option: %s: %s\n",
	"[%s]: ERROR: %s: Invalid command line value for option: %s: %s\n",
	"[%s]: ERROR: %s: Inconsistent configuration values detected: %s: %s\n",
	"[%s]: ERROR: %s: Unable to get current time in microseconds: %s: %s\n",
	"[%s]: ERROR: %s: Unable to initialize the network handlers: %s: %s\n",
	"[%s]: ERROR: %s: Invalid protocol. Must be TCP or UDP: %s: %s\n",
	"[%s]: ERROR: %s: Unable to retrieve an usable address for communication: %s: %s\n",
	"[%s]: ERROR: %s: Unable to set socket option for address reusage: %s: %s\n",
	"[%s]: ERROR: %s: Unable to set socket option for port reusage: %s: %s\n",
	"[%s]: ERROR: %s: Unable to set socket receive timeout value: %s: %s\n",
	"[%s]: ERROR: %s: An error ocurred while sending data to the remote endpoint: %s: %s\n",
	"[%s]: ERROR: %s: An error occured while receiving data from the remote endpoint: %s: %s\n",
	"[%s]: ERROR: %s: An error occured while connecting to the remote endpoint: %s: %s\n",
	"[%s]: ERROR: %s: An error occured while preparing the listenening socket for incoming connections: %s: %s\n",
	"[%s]: ERROR: %s: An error occured while trying to accept for incoming connections: %s: %s\n",
	"[%s]: ERROR: %s: Unable to initialize connector: %s: %s\n",
	"[%s]: ERROR: %s: Unable to initialize listener: %s: %s\n",
	"[%s]: ERROR: %s: Unable to send data to the remote endpoint: %s: %s\n",
	"[%s]: ERROR: %s: Unable to receive data from the remote endpoint: %s: %s\n",
	"[%s]: ERROR: %s: Unable to complete handshake - exchanged values mismatch: %s: %s\n",
	"[%s]: ERROR: %s: Unable to complete handshake stage: %s: %s\n",
	"[%s]: ERROR: %s: Unable to complete negotiation stage: %s: %s\n",
	"[%s]: ERROR: %s: Unable to complete stream stage: %s: %s\n",
	"[%s]: ERROR: %s: Unable to complete report exchange stage: %s: %s\n",
	"[%s]: ERROR: %s: Unable to complete finish stage: %s: %s\n",
	"[%s]: ERROR: %s: Unable to retrieve connection address information: %s: %s\n",
	"[%s]: ERROR: %s: Unable to open file to export JSON report information: %s: %s\n",
	"[%s]: ERROR: %s: Unable to create a new worker: %s: %s\n"
};

static
#if !defined(__GNUC__) && !defined(__clang__)
_Noreturn
#endif
void _error_post_fatal(void)
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((noreturn));
#else
	;
#endif
static void _error_post_nonfatal(void);

static
#if !defined(__GNUC__) && !defined(__clang__)
_Noreturn
#endif
void _error_post_fatal(void) {
	exit(EXIT_FAILURE);

	error_no_return();
}

static void _error_post_nonfatal(void) {
	return;
}

void error_handler(ubwt_error_level_t level, ubwt_error_type_t type, const char *origin_fmt, ...) {
	char time_str[UBWT_CONFIG_CTIME_SIZE] = { 0 }, level_str[16] = { 0 };
	void (*post_process) (void) = &_error_post_fatal;
#ifdef COMPILE_WIN32
	char wsaerr_str[32] = { 0 };
#endif
	char origin[UBWT_ERROR_MSG_SIZE_MAX + 1] = { 0 };
	va_list ap;

	current->error.l_errno = errno;

	if (origin_fmt) {
		va_start(ap, origin_fmt);
		vsnprintf(origin, sizeof(origin) - 1, origin_fmt, ap);
		va_end(ap);
	}

	if (!datetime_now_str(time_str))
		time_str[0] = 0;

	switch (level) {
		case UBWT_ERROR_LEVEL_INFO: {
			strcpy(level_str, "INFO");
			post_process = &_error_post_nonfatal;
		} break;

		case UBWT_ERROR_LEVEL_WARNING: {
			strcpy(level_str, "WARNING");
			post_process = &_error_post_nonfatal;
		} break;

		case UBWT_ERROR_LEVEL_CRITICAL: {
			strcpy(level_str, "CRITICAL");
			post_process = &_error_post_nonfatal;
		} break;

		case UBWT_ERROR_LEVEL_FATAL: {
			strcpy(level_str, "FATAL");
			post_process = &_error_post_fatal;
		}

		default: break;
	}

	if (level >= current->config->error_log_min_level) {
#ifdef COMPILE_WIN32
		if (current->error.l_wsaerr) {
			memset(wsaerr_str, 0, sizeof(wsaerr_str));

			snprintf(wsaerr_str, sizeof(wsaerr_str) - 1, "%ld", current->error.l_wsaerr);

			fprintf(stderr, __errors[type], time_str, level_str, origin[0] ? origin : "unknown_origin", wsaerr_str);
		} else {
			fprintf(stderr, __errors[type], time_str, level_str, origin[0] ? origin : "unknown_origin", current->error.l_eai ? gai_strerror(current->error.l_eai) : strerror(current->error.l_errno));
		}
#else
		fprintf(stderr, __errors[type], time_str, level_str, origin[0] ? origin : "unknown_origin", current->error.l_eai ? gai_strerror(current->error.l_eai) : strerror(current->error.l_errno));
#endif
	}

	post_process();
}

#if !defined(__GNUC__) && !defined(__clang__)
_Noreturn
#endif
void error_abort(const char *file, int line, const char *func) {
	fprintf(stderr, "Program execution will be aborted due to an unrecoverable error state: %s:%d:%s(): %s\n", file, line, func, strerror(errno));

	abort();

	error_no_return();
}

