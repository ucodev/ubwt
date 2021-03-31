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
#include <stdlib.h>

#if !defined(__GNUC__) && !defined(__clang__)
#include <stdnoreturn.h>
#endif

#include "config.h"
#include "error.h"
#include "usage.h"

#if !defined(__GNUC__) && !defined(__clang__)
_Noreturn
#endif
void usage_show(char *const *argv, int success) {
	fprintf(stderr,
		"Usage: %s [OPTIONS] {sender|receiver} ADDRESS\n"
		"\n"
		"OPTIONS\n"
		"\n"
#ifdef UBWT_CONFIG_DEBUG
		"       -d                  Enable debugging.\n"
#endif
		"       -h                  Display this help.\n"
		"       -m <octets>         Link MTU (default: %u octets).\n"
		"       -p <port>           UDP port to listen/connect to (default: %s).\n"
		"       -s <octets>         L4 payload size (default: %u).\n"
		"       -t <seconds>        Minimum stream time (default: %u seconds).\n"
		"       -w <seconds>        Connection timeout (default: %u seconds).\n"
		"\n",
		argv[0],
		UBWT_CONFIG_NET_MTU,
		UBWT_CONFIG_PORT_DEFAULT,
		UBWT_CONFIG_TALK_PAYLOAD_DEFAULT_SIZE,
		UBWT_CONFIG_TALK_STREAM_MINIMUM_TIME,
		UBWT_CONFIG_NET_TIMEOUT_DEFAULT
	);

	exit(success ? EXIT_SUCCESS : EXIT_FAILURE);

	error_no_return();
}

