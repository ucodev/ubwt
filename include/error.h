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

#ifndef UBWT_ERROR_H
#define UBWT_ERROR_H

#include <errno.h>

#if !defined(__GNUC__) && !defined(__clang__)
 #include <stdnoreturn.h>
#endif

#define error_no_return() for (;;)

/* Per-OS errno */
#ifdef ECANCELED
 #define UBWT_ERROR_ABORT	ECANCELED
#else
 #define UBWT_ERROR_ABORT	EAGAIN
#endif

#ifdef EBADE
 #define UBWT_ERROR_MSG_UNEXPECTED	EBADE
#else
 #ifdef ENOMSG
  #define UBWT_ERROR_MSG_UNEXPECTED	ENOMSG
 #else
  #define UBWT_ERROR_MSG_UNEXPECTED	EINVAL
 #endif
#endif

#define UBWT_ERROR_MSG_SIZE_MAX		1024

typedef enum UBWT_ERROR_TYPES {
	UBWT_ERROR_TYPE_NO_ERROR = 0,
	UBWT_ERROR_TYPE_CURRENT_FORK_FAILED,
	UBWT_ERROR_TYPE_CONFIG_ARGV_1_INVALID,
	UBWT_ERROR_TYPE_CONFIG_ARGV_2_INVALID,
	UBWT_ERROR_TYPE_CONFIG_ARGV_OPT_INVALID,
	UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID,
	UBWT_ERROR_TYPE_CONFIG_CONSISTENCY_FAILED,
	UBWT_ERROR_TYPE_TIME_GET,
	UBWT_ERROR_TYPE_NET_INIT,
	UBWT_ERROR_TYPE_NET_INVALID_PROTO,
	UBWT_ERROR_TYPE_NET_ADDRINFO,
	UBWT_ERROR_TYPE_NET_REUSEADDR_FAILED,
	UBWT_ERROR_TYPE_NET_REUSEPORT_FAILED,
	UBWT_ERROR_TYPE_NET_RECV_TIMEO_SET,
	UBWT_ERROR_TYPE_NET_SEND_FAILED,
	UBWT_ERROR_TYPE_NET_RECV_FAILED,
	UBWT_ERROR_TYPE_NET_CONNECT,
	UBWT_ERROR_TYPE_NET_LISTEN,
	UBWT_ERROR_TYPE_NET_ACCEPT,
	UBWT_ERROR_TYPE_TALK_SENDER_INIT,
	UBWT_ERROR_TYPE_TALK_RECEIVER_INIT,
	UBWT_ERROR_TYPE_TALK_SEND_FAILED,
	UBWT_ERROR_TYPE_TALK_RECV_FAILED,
	UBWT_ERROR_TYPE_TALK_HANDSHAKE_NOMATCH,
	UBWT_ERROR_TYPE_TALK_HANDSHAKE_FAILED,
	UBWT_ERROR_TYPE_TALK_NEGOTIATION_FAILED,
	UBWT_ERROR_TYPE_TALK_STREAM_FAILED,
	UBWT_ERROR_TYPE_TALK_REPORT_EXCHANGE_FAILED,
	UBWT_ERROR_TYPE_TALK_FINISH_FAILED,
	UBWT_ERROR_TYPE_REPORT_CONNECTION_FAILED,
	UBWT_ERROR_TYPE_REPORT_JSON_FILE_FAILED,
	UBWT_ERROR_TYPE_WORKER_CREATE_FAILED
} ubwt_error_type_t;

typedef enum UBWT_ERROR_LEVELS {
	UBWT_ERROR_LEVEL_INFO,
	UBWT_ERROR_LEVEL_WARNING,
	UBWT_ERROR_LEVEL_CRITICAL,
	UBWT_ERROR_LEVEL_FATAL
} ubwt_error_level_t;

typedef struct ubwt_error {
	int l_errno;
	int l_eai;
#ifdef COMPILE_WIN32
	int l_wsaerr;
#endif
} ubwt_error_t;

void error_handler(ubwt_error_level_t level, ubwt_error_type_t type, const char *origin_fmt, ...);
#if !defined(__GNUC__) && !defined(__clang__)
_Noreturn
#endif
void error_abort(const char *file, int line, const char *func)
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((noreturn));
#else
	;
#endif


#endif

