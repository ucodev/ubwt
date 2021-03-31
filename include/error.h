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

#ifndef UBWT_ERROR_H
#define UBWT_ERROR_H

#define error_no_return() for (;;)

typedef enum UBWT_ERROR_TYPES {
	UBWT_ERROR_TYPE_NO_ERROR = 0,
	UBWT_ERROR_TYPE_CONFIG_ARGV_1_INVALID,
	UBWT_ERROR_TYPE_CONFIG_ARGV_2_INVALID,
	UBWT_ERROR_TYPE_CONFIG_ARGV_OPT_INVALID,
	UBWT_ERROR_TYPE_TIME_GET,
	UBWT_ERROR_TYPE_NET_INIT,
	UBWT_ERROR_TYPE_NET_ADDRINFO,
	UBWT_ERROR_TYPE_NET_RECV_TIMEO_SET,
	UBWT_ERROR_TYPE_NET_SEND_FAILED,
	UBWT_ERROR_TYPE_NET_RECV_FAILED,
	UBWT_ERROR_TYPE_TALK_SEND_FAILED,
	UBWT_ERROR_TYPE_TALK_RECV_FAILED,
	UBWT_ERROR_TYPE_TALK_HANDSHAKE_FAILED,
	UBWT_ERROR_TYPE_TALK_NEGOTIATION_FAILED,
	UBWT_ERROR_TYPE_TALK_STREAM_FAILED,
	UBWT_ERROR_TYPE_TALK_REPORT_EXCHANGE_FAILED,
	UBWT_ERROR_TYPE_REPORT_CONNECTION_FAILED
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
} ubwt_error_t;

void error_handler(ubwt_error_level_t level, ubwt_error_type_t type, const char *origin);

#endif

