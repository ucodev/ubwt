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
#include "current.h"
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
		"       -A                  Asynchronous bi-directional full-duplex test. TCP only.\n"
#endif
#ifdef UBWT_CONFIG_DEBUG
		"       -d FILE             Append debugging output to a file (default: stderr).\n"
		"       -D                  Enable debugging.\n"
#endif
		"       -b                  Perform a bi-directional test.\n"
		"       -F                  Enable full/extended reporting.\n"
		"       -h                  Display this help.\n"
		"       -I MSEC             Interval between latency measurements (default: %u msec).\n"
		"       -j FILE             Export report in JSON format to file.\n"
		"       -m OCTETS           Link MTU (default: %u octets).\n"
		"       -N ITERATIONS       Number of handshake iterations (default: %u iterations).\n"
#if !defined(UBWT_CONFIG_NET_NO_UDP) && defined(UBWT_CONFIG_NET_USE_SETSOCKOPT)
		"       -p PROTOCOL         L4 protocol: 'tcp' or 'udp' (default: tcp).\n"
#endif
		"       -P PORT             TCP or UDP port to listen/connect to (default: %s).\n"
		"       -r FILE             Store the running PID into the specified file.\n"
		"       -R                  Reverse stream testing first.\n"
		"       -s OCTETS           L4 payload size (default: %u).\n"
		"       -t SECONDS          Minimum stream time (default: %u seconds).\n"
		"       -v                  Display version information.\n"
#ifdef UBWT_CONFIG_NET_USE_SETSOCKOPT
		"       -w SECONDS          Connection timeout (default: %u seconds).\n"
#endif
#ifdef UBWT_CONFIG_MULTI_THREADED
		"       -W COUNT            Maximum number of workers (default: %u). TCP only.\n"
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
		UBWT_CONFIG_TALK_STREAM_MINIMUM_TIME
#ifdef UBWT_CONFIG_NET_USE_SETSOCKOPT
		,
		UBWT_CONFIG_NET_TIMEOUT_DEFAULT
#endif
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

void usage_check_optarg(int opt, char *optarg) {
	FILE *fp = NULL;

	switch (opt) {
		case 'D':
		case 'b':
		case 'F':
		case 'h':
		case 'v': break;

		case 'A': {
			if (current->config->reverse_first) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Option -A cannot be used in combination with option -R");

				error_no_return();
			}

			if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_UDP) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Option -A is not supported when using UDP protocol");

				error_no_return();
			}
		} break;

		case 'd': {
			if (!strlen(optarg)) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -d cannot be empty");

				error_no_return();
			}

			if (!(fp = fopen(optarg, "a+"))) {
				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -d valid file path with write permissions");

				error_no_return();
			} else {
				fclose(fp);
				/* NOTE: do not call unlink(), as the file may already exists before this check
				 * and we don't want to remove it's contents before the report routines are
				 * executed.
				 */
			}
		} break;


		case 'R': {
			if (current->config->asynchronous) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Option -R cannot be used in combination with option -A");

				error_no_return();
			}
		} break;

		case 'I': {
			if (atoi(optarg) < 0 || atoi(optarg) >= 65536) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -I must be between 0 and 65535");

				error_no_return();
			}
		} break;

		case 'j': {
			if (!strlen(optarg)) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -j cannot be empty");

				error_no_return();
			}

			if (!(fp = fopen(optarg, "w+"))) {
				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -j valid file path with write permissions");

				error_no_return();
			} else {
				fclose(fp);
				/* NOTE: do not call unlink(), as the file may already exists before this check
				 * and we don't want to remove it's contents before the report routines are
				 * executed.
				 */
			}
		} break;

		case 'N': {
			if (atoi(optarg) <= 0 || atoi(optarg) >= 65536) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -N must be between 1 and 65535");

				error_no_return();
			}
		} break;

		case 'm': {
			if (atoi(optarg) < UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE || atoi(optarg) >= 65536) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -m must be between 508 and 65535"); /* TODO: Implement string fmt on error_handler() */

				error_no_return();
			}
		} break;

		case 'p': {
			if (strlen(optarg) >= sizeof(current->config->net_l4_proto_name)) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -p is too long. Must be \'tcp\' or \'udp\'");

				error_no_return();
			}

			if (strcmp(optarg, "tcp") && strcmp(optarg, "udp")) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -p is invalid. Must be \'tcp\' or \'udp\'");

				error_no_return();
			}
		} break;

		case 'P': {
			if (atoi(optarg) <= 0 || atoi(optarg) >= 65536) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -P must be between 1 and 65535");

				error_no_return();
			}
		} break;

		case 'r': {
			if (!strlen(optarg)) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -r cannot be empty");

				error_no_return();
			}

			if (!(fp = fopen(optarg, "w+"))) {
				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -r valid file path with write permissions");

				error_no_return();
			} else {
				fclose(fp);
				/* NOTE: do not call unlink(), as the file may already exists before this check
				 * and we don't want to remove it's contents before the runtime routines actually
				 * modify its contents.
				 */
			}
		} break;

		case 's': {
			if (atoi(optarg) < UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE || atoi(optarg) >= 65536) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -s must be between 508 and 65535"); /* TODO: Implement string fmt on error_handler() */

				error_no_return();
			}
		} break;

		case 't': {
			if (atoi(optarg) <= 0 || atoi(optarg) >= 65536) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -t must be between 1 and 65535");

				error_no_return();
			}
		} break;

#ifdef UBWT_CONFIG_NET_USE_SETSOCKOPT
		case 'w': {
			if (atoi(optarg) <= 0 || atoi(optarg) >= 65536) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -w must be between 1 and 65535");

				error_no_return();
			}
		} break;
#endif

		case 'W': {
			if (atoi(optarg) <= 0 || atoi(optarg) > 1024) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -W must be between 1 and 1024");

				error_no_return();
			}

			if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_UDP && atoi(optarg) > 1) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -W must be 1 when using UDP protocol");

				error_no_return();
			}
		} break;

		default: error_abort(__FILE__, __LINE__, "usage_check_optarg");
	}
}

