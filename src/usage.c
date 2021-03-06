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
#include <unistd.h>

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
		"       -b                  Perform a bi-directional test.\n"
		"       -B OCTETS           Size of the round-robin buffer used for payload. Requires -C.\n"
		"       -C FILE             Creates a buffer from file to fill the payload. Requires -B.\n"
#ifdef UBWT_CONFIG_DEBUG
		"       -d FILE             Append debugging output to a file (default: stderr).\n"
		"       -D                  Enable debugging.\n"
#endif
		"       -F                  Enable full/extended reporting.\n"
		"       -h                  Display this help.\n"
		"       -I MSEC             Interval between latency measurements (default: %u msec).\n"
		"       -j FILE             Export report in JSON format to file.\n"
		"       -m OCTETS           Link MTU (default: %u octets).\n"
		"       -M MULTIPLIER       Talk count multiplier (default: auto).\n"
		"       -N ITERATIONS       Number of handshake iterations (default: %u iterations).\n"
		"       -o SECONDS          Process reverse delay. Sets -b (default: %u seconds).\n"
#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
		"       -O SECONDS          Maximum wait time before next stream (default: %u seconds).\n"
#endif
#ifndef UBWT_CONFIG_NET_NO_UDP
		"       -p PROTOCOL         L4 protocol: 'tcp' or 'udp' (default: tcp).\n"
#endif
		"       -P PORT             TCP or UDP port to listen/connect to (default: %s).\n"
		"       -r FILE             Store the running PID into the specified file.\n"
		"       -R                  Reverse stream testing first.\n"
		"       -s OCTETS           L4 payload size (default: %u).\n"
		"       -t SECONDS          Minimum stream time (default: %u seconds).\n"
#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
		"       -T SECONDS          Maximum wait time between stream packets (default: %u seconds).\n"
#endif
		"       -v                  Display version information.\n"
#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
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
		UBWT_CONFIG_PROCESS_REVERSE_DELAY_SEC,
#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
		UBWT_CONFIG_NET_TIMEOUT_TALK_STREAM_END,
#endif
		UBWT_CONFIG_PORT_DEFAULT,
		UBWT_CONFIG_TALK_PAYLOAD_DEFAULT_SIZE,
		UBWT_CONFIG_TALK_STREAM_MINIMUM_TIME
#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
		,
		UBWT_CONFIG_NET_TIMEOUT_TALK_STREAM_RUN,
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

		case 'B': {
			if (atol(optarg) < 0 || atol(optarg) > 2147483647) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -B must be between 0 and 2147483647");

				error_no_return();
			}
		} break;

		case 'C': {
			if (!strlen(optarg)) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -C cannot be empty");

				error_no_return();
			}

			if (!(fp = fopen(optarg, "r"))) {
				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -C valid file path with read permissions");

				error_no_return();
			} else {
				fclose(fp);
			}
		} break;

		case 'd': {
			if (!strlen(optarg)) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -d cannot be empty");

				error_no_return();
			}

			if (!strcmp(optarg, "stderr") || !strcmp(optarg, "stdout"))
				break;

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

		case 'm': {
			if (atoi(optarg) < UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE || atoi(optarg) >= 65536) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -m must be between %u and 65535", UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE);

				error_no_return();
			}
		} break;

		case 'M': {
			if (atoi(optarg) <= 1 || atoi(optarg) >= UBWT_CONFIG_TALK_COUNT_MUL_MAX) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -M must be greater than 1 and less than %u", UBWT_CONFIG_TALK_COUNT_MUL_MAX);

				error_no_return();
			}
		} break;

		case 'N': {
			if (atoi(optarg) <= 0 || atoi(optarg) >= 65536) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -N must be between 1 and 65535");

				error_no_return();
			}
		} break;

		case 'o': {
			if (atoi(optarg) < 1 || atoi(optarg) >= 65536) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -o must be between 1 and 65535");

				error_no_return();
			}
		} break;

#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
		case 'O': {
			if (atoi(optarg) < 1 || atoi(optarg) >= 65536) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -O must be between 1 and 65535");

				error_no_return();
			}
		} break;
#endif
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

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -s must be between %u and 65535", UBWT_CONFIG_TALK_PAYLOAD_MIN_SIZE);

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

#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
		case 'T': {
			if (atoi(optarg) <= 0 || atoi(optarg) >= 65536) {
				errno = EINVAL;

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPTARG_INVALID, "usage_check_optarg(): Value for -T must be between 1 and 65535");

				error_no_return();
			}
		} break;

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

		case '?': {
			errno = EINVAL;

			error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPT_INVALID, "usage_check_optarg(): An unrecognized command-line option was used (-%c)", optopt);

			error_no_return();
		} break;

		default: {
			error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_CONFIG_ARGV_OPT_INVALID, "usage_cehck_optarg(): Unknown value returned by getopt(): 0%o", opt);

			error_no_return();
		}
	}
}

