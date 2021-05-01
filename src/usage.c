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
		"Usage: %s [OPTIONS] {connector|listener} {HOSTNAME|IPv4|IPv6}\n"
		"\n"
		"OPTIONS\n"
		"\n"
#ifdef UBWT_CONFIG_MULTI_THREADED
		"       -A                  Perform an asynchronous bi-directional full-duplex test.\n"
#endif
#ifdef UBWT_CONFIG_DEBUG
		"       -d                  Enable debugging.\n"
#endif
		"       -b                  Perform a bi-directional test.\n"
		"       -F                  Enable full/extended reporting.\n"
		"       -h                  Display this help.\n"
		"       -I <msec>           Interval between latency measurements (default: %u msec).\n"
		"       -m <octets>         Link MTU (default: %u octets).\n"
		"       -N <iterations>     Number of handshake iterations (default: %u iterations).\n"
#if !defined(UBWT_CONFIG_NET_NO_UDP) && defined(UBWT_CONFIG_NET_USE_SETSOCKOPT)
		"       -p <protocol>       L4 protocol: 'tcp' or 'udp' (default: tcp).\n"
#endif
		"       -P <port>           TCP or UDP port to listen/connect to (default: %s).\n"
		"       -s <octets>         L4 payload size (default: %u).\n"
		"       -t <seconds>        Minimum stream time (default: %u seconds).\n"
		"       -v                  Display version information.\n"
		"       -w <seconds>        Connection timeout (default: %u seconds).\n"
#ifdef UBWT_CONFIG_MULTI_THREADED
		"       -W <count>          Maximum number of workers (default: %u).\n"
#endif
		"\n"
		"EXAMPLES\n"
		"\n"
		"       Unidirectional test :      im@listening ~ $ ubwt listener myaddress.local\n"
		"                           :  im_the@connector ~ $ ubwt connector listening.local\n"
		"\n"
		"       Bidirectional test  :  term_1@localhost ~ $ ubwt -b listener 127.0.0.1\n"
		"                           :  term_2@localhost ~ $ ubwt -b connector 127.0.0.1\n"
		"\n",
		argv[0],
		UBWT_CONFIG_TALK_HANDSHAKE_INTERVAL_MSEC,
		UBWT_CONFIG_NET_MTU,
		UBWT_CONFIG_TALK_HANDSHAKE_ITER,
		UBWT_CONFIG_PORT_DEFAULT,
		UBWT_CONFIG_TALK_PAYLOAD_DEFAULT_SIZE,
		UBWT_CONFIG_TALK_STREAM_MINIMUM_TIME,
		UBWT_CONFIG_NET_TIMEOUT_DEFAULT
#ifdef UBWT_CONFIG_MULTI_THREADED
		,
		UBWT_CONFIG_PROCESS_WORKER_COUNT
#endif
	);

	exit(success ? EXIT_SUCCESS : EXIT_FAILURE);

	error_no_return();
}

#if !defined(__GNUC__) && !defined(__clang__)
_Noreturn
#endif
void usage_version(void) {
	fprintf(stdout, "ubwt (uCodev Bandwidth Tester) %s\n"
		"Copyright (C) 2021 Pedro A. Hortas <pah at ucodev dot org>\n", UBWT_CONFIG_VERSION_STR);
	fprintf(stdout, "This is free software; see the source for copying conditions.  There is NO\n"
		"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n");

	exit(EXIT_SUCCESS);

	error_no_return();
}

