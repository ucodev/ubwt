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
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <sys/types.h>

#include "bitops.h"
#include "config.h"
#include "current.h"
#include "datetime.h"
#include "debug.h"
#include "error.h"
#include "net.h"
#include "report.h"
#include "stage.h"
#include "talk.h"

static const char *__talk_ops_str[] = {
	"UBWT_TALK_OP_NOOP",
	"UBWT_TALK_OP_PING",
	"UBWT_TALK_OP_PONG",
	"UBWT_TALK_OP_LATENCY_US",
	"UBWT_TALK_OP_COUNT_REQ",
	"UBWT_TALK_OP_STREAM_START",
	"UBWT_TALK_OP_STREAM_RUN",
	"UBWT_TALK_OP_STREAM_END",
	"UBWT_TALK_OP_STREAM_WEAK",
	"UBWT_TALK_OP_REPORT",
	"UBWT_TALK_OP_FINISH",
	"UBWT_TALK_OP_FORCE_FAIL"
};

unsigned __talk_weak = 0;
struct ubwt_talk_context __talk[2] = { { &__talk_weak, 0, 0 }, { &__talk_weak, 0, 0 } };

const char *talk_op_to_str(ubwt_talk_ops_t op) {
	return __talk_ops_str[op];
}

static void _talk_timeout(time_t timeout) {
	if (net_timeout_set(current->net.fd, timeout) < 0) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_NET_RECV_TIMEO_SET, "_talk_timeout(): net_timeout_set()");
		error_no_return();
	}
}

static int _talk_weak_stream_needs_restart(void) {
#ifdef UBWT_CONFIG_MULTI_THREADED
	if (current->config->asynchronous) {
		if (current->talk[0].im_weak == (current->config->worker_count / 2) || current->talk[1].im_weak == (current->config->worker_count / 2))
			return 1;
	} else if (current->talk[process_get_reverse()].im_weak == current->config->worker_count) {
		return 1;
	}

	return 0;
#else
	return current->talk[process_get_reverse()].im_weak;
#endif
}

#ifdef UBWT_CONFIG_MULTI_THREADED
static int _talk_weak_stream_needs_recount(void) {
	return current->talk[process_get_reverse()].im_weak == (current->config->worker_count / (1 + current->config->asynchronous));
}
#endif

static ssize_t _talk_send(const ubwt_talk_payload_t *pkt) {
	size_t len = 0, hdr_size = offsetof(ubwt_talk_payload_t, buf);
	ubwt_talk_payload_t pktbuf = { 0, 0, 0, 0, 0, 0, { 0 } };

	if (stage_get() == UBWT_STAGE_STATE_RUNTIME_TALK_HANDSHAKE) {
		len = hdr_size;
	} else {
		len = current->config->talk_payload_current_size;
		memcpy(((char *) &pktbuf) + hdr_size, ((const char *) pkt) + hdr_size, len - hdr_size);
	}

	pktbuf.flags = htonl(pkt->flags);
	pktbuf.count = htonl(pkt->count);
	pktbuf.latency = htonl(pkt->latency);
	pktbuf.weak_time = net_htonll(pkt->weak_time);
#ifdef UBWT_CONFIG_MULTI_THREADED
	pktbuf.worker_id = net_htonll((uint64_t) worker_self());
#endif

	return net_im_connector() ?
		net_write_to_listener(&pktbuf, len) :
		net_write_to_connector(&pktbuf, len);
}

static ssize_t _talk_recv(ubwt_talk_payload_t *pkt) {
	ssize_t ret = 0, len = 0, hdr_size = offsetof(ubwt_talk_payload_t, buf);

	len = stage_get() == UBWT_STAGE_STATE_RUNTIME_TALK_HANDSHAKE ? hdr_size : current->config->talk_payload_current_size;

	ret = net_im_connector() ?
		net_read_from_listener(pkt, len) :
		net_read_from_connector(pkt, len);

	if (ret == len) {
		pkt->flags = ntohl(pkt->flags);
		pkt->count = ntohl(pkt->count);
		pkt->latency = ntohl(pkt->latency);
		pkt->weak_time = net_ntohll(pkt->weak_time);
#ifdef UBWT_CONFIG_MULTI_THREADED
		pkt->worker_id = net_ntohll(pkt->worker_id);
		current->remote_worker_id = (ubwt_worker_t) pkt->worker_id;
#endif
	}

	/* Force receive to fail if the remote host indicates it */
	if (bit_test(&pkt->flags, UBWT_TALK_OP_FORCE_FAIL)) {
		debug_info_talk_op(UBWT_TALK_OP_FORCE_FAIL, "RECV");

		errno = UBWT_ERROR_ABORT;

		return -1;
	}

	return ret;
}

static void _talk_send_abort(void) {
	int errsv = errno;
	ubwt_talk_payload_t p = { 0, 0, 0, 0, 0, 0, { 0 } };

	debug_info_talk_op(UBWT_TALK_OP_FORCE_FAIL, "SEND");

	bit_set(&p.flags, UBWT_TALK_OP_FORCE_FAIL);

	if (_talk_send(&p) > 0) {
		debug_info_talk_op(UBWT_TALK_OP_FORCE_FAIL, "SENT");
	} else {
		debug_info_talk_op(UBWT_TALK_OP_FORCE_FAIL, "FAIL");
	}

	if (errsv)
		errno = errsv; /* Preserve original errno */
}

static long _talk_sender_handshake(void) {
	ubwt_talk_payload_t p = { 0, current->config->talk_handshake_iter, 0, 0, 0, 0, { 0 } };
	uint64_t dt = datetime_now_us();


	/* Send PING op */

	bit_set(&p.flags, UBWT_TALK_OP_PING);

	debug_info_talk_op(UBWT_TALK_OP_PING, "SEND");

	if (_talk_send(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_PING, "FAIL");

		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_SEND_FAILED, "_talk_sender_handshake(): _talk_send()");
		return -1L;
	}

	debug_info_talk_op(UBWT_TALK_OP_PING, "SENT");


	/* Wait for PONG op reply */

	debug_info_talk_op(UBWT_TALK_OP_PONG, "WAIT");

	if (_talk_recv(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_PONG, "FAIL");

		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_RECV_FAILED, "_talk_sender_handshake(): _talk_recv()");
		return -1L;
	}

	if (bit_test(&p.flags, UBWT_TALK_OP_PONG)) {
		debug_info_talk_op(UBWT_TALK_OP_PONG, "RECV");
	} else {
		debug_info_talk_op(UBWT_TALK_OP_PONG, "MISS");
	}

	if (p.count != current->config->talk_handshake_iter) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_HANDSHAKE_NOMATCH, "_talk_sender_handshake(): Iteration count doesn't match between the two parties");
		errno = EINVAL;

		return -1L;
	}

	dt = datetime_now_us() - dt;

	return (long) dt;
}

static int _talk_receiver_handshake(void) {
	ubwt_talk_payload_t p = { 0, 0, 0, 0, 0, 0, { 0 } };


	/* Wait for PING op */

	debug_info_talk_op(UBWT_TALK_OP_PING, "WAIT");

	if (_talk_recv(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_PING, "FAIL");

		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_RECV_FAILED, "_talk_receiver_handshake(): _talk_recv()");
		return -1;
	}

	if (bit_test(&p.flags, UBWT_TALK_OP_PING)) {
		debug_info_talk_op(UBWT_TALK_OP_PING, "RECV");
	} else {
		debug_info_talk_op(UBWT_TALK_OP_PING, "MISS");

		errno = UBWT_ERROR_MSG_UNEXPECTED;

		return -1;
	}

	if (p.count != current->config->talk_handshake_iter) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_HANDSHAKE_NOMATCH, "_talk_receiver_handshake(): Iteration count doesn't match between the two parties");
		errno = EINVAL;

		return -1;
	}

	memset(&p, 0, sizeof(p));

	bit_set(&p.flags, UBWT_TALK_OP_PONG);

	p.count = current->config->talk_handshake_iter;


	/* Send PONG op reply */

	debug_info_talk_op(UBWT_TALK_OP_PONG, "SEND");

	if (_talk_send(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_PONG, "FAIL");

		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_SEND_FAILED, "_talk_receiver_handshake(): _talk_send()");

		return -1;
	}

	debug_info_talk_op(UBWT_TALK_OP_PONG, "SENT");

	return 0;
}

static int _talk_sender_negotiate(uint32_t count, uint32_t latency) {
	ubwt_talk_payload_t p = { 0, 0, 0, 0, 0, 0, { 0 } };

	bit_set(&p.flags, UBWT_TALK_OP_COUNT_REQ);
	bit_set(&p.flags, UBWT_TALK_OP_LATENCY_US);
	p.count = count;
	p.latency = latency;


	/* Send COUNT and LATENCY data to receiver */

	debug_info_talk_op(UBWT_TALK_OP_COUNT_REQ, "SEND");
	debug_info_talk_op(UBWT_TALK_OP_LATENCY_US, "SEND");

	if (_talk_send(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_COUNT_REQ, "FAIL");
		debug_info_talk_op(UBWT_TALK_OP_LATENCY_US, "FAIL");

		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_SEND_FAILED, "_talk_sender_negotiate(): _talk_send()");

		return -1;
	}

	debug_info_talk_op(UBWT_TALK_OP_COUNT_REQ, "SENT");
	debug_info_talk_op(UBWT_TALK_OP_LATENCY_US, "SENT");

	return 0;
}

static int _talk_receiver_negotiate(uint32_t *count, uint32_t *latency) {
	ubwt_talk_payload_t p = { 0, 0, 0, 0, 0, 0, { 0 } };


	/* Get COUNT and LATENCY data from sender */

	debug_info_talk_op(UBWT_TALK_OP_COUNT_REQ, "WAIT");
	debug_info_talk_op(UBWT_TALK_OP_LATENCY_US, "WAIT");

	if (_talk_recv(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_COUNT_REQ, "FAIL");
		debug_info_talk_op(UBWT_TALK_OP_LATENCY_US, "FAIL");

		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_RECV_FAILED, "_talk_receiver_negotiate(): _talk_recv()");

		return -1;
	}


	/* Check if COUNT is set and store it */

	if (bit_test(&p.flags, UBWT_TALK_OP_COUNT_REQ)) {
		debug_info_talk_op(UBWT_TALK_OP_COUNT_REQ, "RECV");

		if (p.count < UBWT_CONFIG_TALK_COUNT_MAX) {
			*count = p.count;
		} else {
			debug_info_talk_op(UBWT_TALK_OP_COUNT_REQ, "FAIL");

			errno = EINVAL;

			return -1;
		}
	} else {
		debug_info_talk_op(UBWT_TALK_OP_COUNT_REQ, "MISS");

		errno = UBWT_ERROR_MSG_UNEXPECTED;

		return -1;
	}


	/* Check if LATENCY is set and store it */

	if (bit_test(&p.flags, UBWT_TALK_OP_LATENCY_US)) {
		debug_info_talk_op(UBWT_TALK_OP_LATENCY_US, "RECV");

		*latency = p.latency;
	} else {
		debug_info_talk_op(UBWT_TALK_OP_LATENCY_US, "MISS");

		errno = UBWT_ERROR_MSG_UNEXPECTED;

		return -1;
	}

	return 0;
}

static int _talk_sender_stream(uint32_t count) {
	uint32_t i = 0;
	ubwt_talk_payload_t p = { 0, 0, 0, 0, 0, 0, { 0 } };
	uint32_t payload_size = current->config->talk_payload_current_size; /* Local, to cut current access time */
	int payload_fill = current->config->net_payload_buffer_size ? 1 : 0; /* Local, to cut current access time */

	/* Wait for STREAM START op */

	debug_info_talk_op(UBWT_TALK_OP_STREAM_START, "WAIT");

	if (_talk_recv(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_STREAM_START, "FAIL");

		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_RECV_FAILED, "_talk_sender_stream(): _talk_recv()");

		return -1;
	}

	debug_info_talk_op(UBWT_TALK_OP_STREAM_START, "RECV");

	if (bit_test(&p.flags, UBWT_TALK_OP_STREAM_START)) {

		/* Start STREAM RUN op - send the data to the remote host */

		memset(&p, 0, sizeof(p));

		bit_set(&p.flags, UBWT_TALK_OP_STREAM_RUN);

#ifdef UBWT_CONFIG_MULTI_THREADED
		/* Syncronize workers - context reset */

		if (current->config->asynchronous) {
			/* Sync sending/receiving workers */
			worker_barrier_wait(&current->worker_barrier_global[2]);
		} else {
			/* Sync sending workers */
			worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
		}

		worker_mutex_lock(current->worker_mutex_global);
#endif

		*current->talk[process_get_reverse()].weak = 0;
		current->talk[process_get_reverse()].im_weak = 0;
		current->talk[process_get_reverse()].weak_time = 0;

#ifdef UBWT_CONFIG_MULTI_THREADED
		worker_mutex_unlock(current->worker_mutex_global);

		/* Syncronize workers - STREAM START */

		if (current->config->asynchronous) {
			/* In asynchronous mode, wait for both straight and reverse
			 * workers ta start transmitting at the same time
			 */
			worker_barrier_wait(&current->worker_barrier_global[2]);
		} else {
			worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
		}
#endif

		_talk_timeout(current->config->net_timeout_talk_stream_run);

		debug_info_talk_op(UBWT_TALK_OP_STREAM_RUN, "SEND");

		for (i = 0; i < count; i ++, p.count ++) {
			if (payload_fill)
				net_buf_fill(p.buf, payload_size);

			if (_talk_send(&p) < 0) {
				debug_info_talk_op(UBWT_TALK_OP_STREAM_RUN, "FAIL");

				error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_SEND_FAILED, "_talk_sender_stream(): _talk_send()");

				return -1;
			}
		}

		_talk_timeout(current->config->net_timeout_talk_stream_end);

#ifdef UBWT_CONFIG_MULTI_THREADED
		/* Syncronize sending workers - STREAM END */
		worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
#endif

		debug_info_talk_op(UBWT_TALK_OP_STREAM_RUN, "SENT");
	} else {
		errno = UBWT_ERROR_MSG_UNEXPECTED;

		return -1;
	}

#ifdef UBWT_CONFIG_MULTI_THREADED
	/* If in asynchronous mode, only syncronize all workers (senders and receivers)
	 * after the sending workers report stream success ("SENT") - STREAM END
	 */
	if (current->config->asynchronous)
		worker_barrier_wait(&current->worker_barrier_global[2]);
#endif

	/* Wait for the remote host to send STREAM END */

	debug_info_talk_op(UBWT_TALK_OP_STREAM_END, "WAIT");

	if (_talk_recv(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_STREAM_END, "FAIL");

		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_RECV_FAILED, "_talk_sender_stream(): _talk_recv()");

		return -1;
	}

	if (bit_test(&p.flags, UBWT_TALK_OP_STREAM_END)) {
		debug_info_talk_op(UBWT_TALK_OP_STREAM_END, "RECV");
	} else {
		debug_info_talk_op(UBWT_TALK_OP_STREAM_END, "MISS");

		errno = UBWT_ERROR_MSG_UNEXPECTED;

		return -1;
	}


	/* Check if the stream was considered WEAK by the remote host */

	if (bit_test(&p.flags, UBWT_TALK_OP_STREAM_WEAK))
		debug_info_talk_op(UBWT_TALK_OP_STREAM_WEAK, "RECV");

	_talk_timeout(current->config->net_timeout_default);

#ifdef UBWT_CONFIG_MULTI_THREADED
	/* Wait for all workers to sync before setting current->talk[process_get_reverse()].weak */

	if (current->config->asynchronous) {
		worker_barrier_wait(&current->worker_barrier_global[2]);
	} else {
		worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
	}
#endif

	/* Set the weak global value if at least one worker had a weak stream */

	if (bit_test(&p.flags, UBWT_TALK_OP_STREAM_WEAK)) {
#ifdef UBWT_CONFIG_MULTI_THREADED
		worker_mutex_lock(current->worker_mutex_global);
#endif

		(*current->talk[process_get_reverse()].weak) ++;
		current->talk[process_get_reverse()].im_weak ++;
		current->talk[process_get_reverse()].weak_time = p.weak_time;

#ifdef UBWT_CONFIG_MULTI_THREADED
		worker_mutex_unlock(current->worker_mutex_global);
#endif
	}

#ifdef UBWT_CONFIG_MULTI_THREADED
	/* Wait for all workers to sync before returning current->talk[process_get_reverse()].weak */

	if (current->config->asynchronous) {
		worker_barrier_wait(&current->worker_barrier_global[2]);
	} else {
		worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
	}
#endif

	/* Return 1 if at least one of the worker streams in this context is WEAK, otherwise 0 */

	return *current->talk[process_get_reverse()].weak > 0;
}

static int _talk_receiver_stream(uint32_t count) {
	ssize_t ret = 0;
	uint32_t i = 0;
	uint64_t t = 0, len = 0;
	ubwt_talk_payload_t p = { 0, 0, 0, 0, 0, 0, { 0 } };


	/* Send STREAM START to remote host */

	bit_set(&p.flags, UBWT_TALK_OP_STREAM_START);
	p.count = count;

	debug_info_talk_op(UBWT_TALK_OP_STREAM_START, "SEND");

	if (_talk_send(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_STREAM_START, "FAIL");

		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_SEND_FAILED, "_talk_receiver_stream(): _talk_send()");

		return -1;
	}

	debug_info_talk_op(UBWT_TALK_OP_STREAM_START, "SENT");

	memset(&p, 0, sizeof(p));


	/* Wait for STREAM RUN to start */

	debug_info_talk_op(UBWT_TALK_OP_STREAM_RUN, "WAIT");

#ifdef UBWT_CONFIG_MULTI_THREADED
	/* Syncronize workers - context reset */

	if (current->config->asynchronous) {
		/* Sync sending/receiving workers */
		worker_barrier_wait(&current->worker_barrier_global[2]);
	} else {
		/* Sync receiving workers */
		worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
	}

	worker_mutex_lock(current->worker_mutex_global);
#endif

	*current->talk[process_get_reverse()].weak = 0;
	current->talk[process_get_reverse()].im_weak = 0;
	current->talk[process_get_reverse()].weak_time = 0;

#ifdef UBWT_CONFIG_MULTI_THREADED
	worker_mutex_unlock(current->worker_mutex_global);

	/* Syncronize workers - STREAM START */

	if (current->config->asynchronous) {
		/* In asynchronous mode, wait for both straight and reverse
		 * workers ta start transmitting at the same time
		 */
		worker_barrier_wait(&current->worker_barrier_global[2]);
	} else {
		worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
	}
#endif

	/* Retrieve start time */
	t = datetime_now_us();

	report_talk_stream_time_start_set(t);

	_talk_timeout(current->config->net_timeout_talk_stream_run);

	for (i = 0; i < count; i ++) {
		if ((ret = _talk_recv(&p)) < 0) {
#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
			if (errno == EAGAIN)
				break; /* NOTE: Timeout (likely...) */
#endif
			/* This is a fatal error and should abort program execution to avoid deadlocks */
			error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_TALK_RECV_FAILED, "_talk_receiver_stream(): _talk_recv()");

			error_no_return();
		}

		len += ret;
	}

	_talk_timeout(current->config->net_timeout_talk_stream_end);

	debug_info_talk_op(UBWT_TALK_OP_STREAM_RUN, "RECV");

#ifdef UBWT_CONFIG_MULTI_THREADED
	/* Syncronize receiving workers - STREAM END */
	worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
#endif

	/* Compute transmission time - if the expected count packets are not received due a timeout, subtract the stream run timeout value */
	t = datetime_now_us() - t - ((i != count) ? (current->config->net_timeout_talk_stream_run * 1000000) : 0);

	assert(t < current_time_elapsed());


#ifdef UBWT_CONFIG_MULTI_THREADED
	/* If in asynchronous mode, only syncronize all workers (senders and receivers)
	 * after calculating the stream time for the receiving workers - STREAM END
	 */
	if (current->config->asynchronous)
		worker_barrier_wait(&current->worker_barrier_global[2]);
#endif

	/* Store report data */

	report_talk_stream_time_duration_set(t);
	report_talk_stream_recv_pkts_set(i);
	report_talk_stream_recv_bytes_set(len);


	/* Send STREAM END */

	debug_info_talk_op(UBWT_TALK_OP_STREAM_END, "SEND");

	memset(&p, 0, sizeof(p));
	bit_set(&p.flags, UBWT_TALK_OP_STREAM_END);

	/* Check if the STREAM was WEAK - if so, mark the STREAM END as WEAK */
	if (t < (current->config->talk_stream_minimum_time * 1000000)) {
		debug_info_talk_op(UBWT_TALK_OP_STREAM_WEAK, "SEND");

		p.weak_time = t;
		bit_set(&p.flags, UBWT_TALK_OP_STREAM_WEAK);
	}

	if (_talk_send(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_STREAM_END, "FAIL");

		if (bit_test(&p.flags, UBWT_TALK_OP_STREAM_WEAK))
			debug_info_talk_op(UBWT_TALK_OP_STREAM_WEAK, "FAIL");

		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_SEND_FAILED, "_talk_receiver_stream(): _talk_send()");

		return -1;
	}

	debug_info_talk_op(UBWT_TALK_OP_STREAM_END, "SENT");

	if (bit_test(&p.flags, UBWT_TALK_OP_STREAM_WEAK))
		debug_info_talk_op(UBWT_TALK_OP_STREAM_WEAK, "SENT");

	_talk_timeout(current->config->net_timeout_default);

#ifdef UBWT_CONFIG_MULTI_THREADED
	/* Wait for all workers to sync before setting current->talk[process_get_reverse()].weak */
	if (current->config->asynchronous) {
		worker_barrier_wait(&current->worker_barrier_global[2]);
	} else {
		worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
	}
#endif

	/* Set the weak global value if at least one worker had a weak stream */
	if (bit_test(&p.flags, UBWT_TALK_OP_STREAM_WEAK)) {
#ifdef UBWT_CONFIG_MULTI_THREADED
		worker_mutex_lock(current->worker_mutex_global);
#endif

		(*current->talk[process_get_reverse()].weak) ++;
		current->talk[process_get_reverse()].im_weak ++;
		current->talk[process_get_reverse()].weak_time = t;

#ifdef UBWT_CONFIG_MULTI_THREADED
		worker_mutex_unlock(current->worker_mutex_global);
#endif
	}

#ifdef UBWT_CONFIG_MULTI_THREADED
	/* Wait for all workers to sync before returning current->talk[process_get_reverse()].weak */
	if (current->config->asynchronous) {
		worker_barrier_wait(&current->worker_barrier_global[2]);
	} else {
		worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
	}
#endif

	/* Return 1 if at least one of the worker streams in this context is WEAK, otherwise 0 */

	return *current->talk[process_get_reverse()].weak > 0;
}

static int _talk_sender_report_exchange(void) {
	ubwt_talk_payload_t p = { 0, 0, 0, 0, 0, 0, { 0 } };


	/* Wait for REPORT */

	debug_info_talk_op(UBWT_TALK_OP_REPORT, "WAIT");

	if (_talk_recv(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_REPORT, "FAIL");

		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_RECV_FAILED, "_talk_sender_report_exchange(): _talk_recv()");

		return -1;
	}

	if (bit_test(&p.flags, UBWT_TALK_OP_REPORT)) {
		debug_info_talk_op(UBWT_TALK_OP_REPORT, "RECV");

		report_unmarshal(p.buf, current->config->talk_payload_current_size);
	} else {
		debug_info_talk_op(UBWT_TALK_OP_REPORT, "MISS");

		errno = UBWT_ERROR_MSG_UNEXPECTED;

		return -1;
	}

	return 0;
}

static int _talk_receiver_report_exchange(void) {
	ubwt_talk_payload_t p = { 0, 0, 0, 0, 0, 0, { 0 } };


	/* Send REPORT */

	debug_info_talk_op(UBWT_TALK_OP_REPORT, "SEND");

	bit_set(&p.flags, UBWT_TALK_OP_REPORT);

	report_marshal(p.buf, current->config->talk_payload_current_size);

	if (_talk_send(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_REPORT, "FAIL");

		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_TALK_SEND_FAILED, "_talk_receiver_report_exchange(): _talk_send()");

		return -1;
	}

	debug_info_talk_op(UBWT_TALK_OP_REPORT, "SENT");

	return 0;
}

static int _talk_sender_finish(void) {
	ubwt_talk_payload_t p = { 0, 0, 0, 0, 0, 0, { 0 } };


	/* Send FINISH */

	debug_info_talk_op(UBWT_TALK_OP_FINISH, "SEND");

	bit_set(&p.flags, UBWT_TALK_OP_FINISH);

	if (_talk_send(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_FINISH, "FAIL");

		/* Not critical */
		error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_TALK_SEND_FAILED, "_talk_sender_finish(): _talk_send()");

		return -1;
	}

	debug_info_talk_op(UBWT_TALK_OP_FINISH, "SENT");

	return 0;
}

static int _talk_receiver_finish(void) {
	ubwt_talk_payload_t p = { 0, 0, 0, 0, 0, 0, { 0 } };


	/* Wait for FINISH */

	debug_info_talk_op(UBWT_TALK_OP_FINISH, "WAIT");

	if (_talk_recv(&p) < 0) {
		debug_info_talk_op(UBWT_TALK_OP_FINISH, "FAIL");

		/* Not critical */
		error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_TALK_RECV_FAILED, "_talk_receiver_finish(): _talk_recv()");

		return -1;
	}

	if (bit_test(&p.flags, UBWT_TALK_OP_FINISH)) {
		debug_info_talk_op(UBWT_TALK_OP_FINISH, "RECV");
	} else {
		/* Not critical */
		debug_info_talk_op(UBWT_TALK_OP_FINISH, "MISS");

		errno = UBWT_ERROR_MSG_UNEXPECTED;

		return -1;
	}

	return 0;
}

void talk_sender(void) {
	int i = 0, ret = 0;
	uint32_t dt = 0;
	uint64_t t = 0;
	double mul = 0;

	stage_set(UBWT_STAGE_STATE_RUNTIME_TALK_HANDSHAKE, 0);

#ifdef UBWT_CONFIG_MULTI_THREADED
	/* We need to sync all workers after talk_init(), and before any transmission, so all init values are syncd */
	if (current->config->asynchronous) {
		worker_barrier_wait(&current->worker_barrier_global[2]);
	} else {
		worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
	}
#endif

	for (i = 0; i < current->config->talk_handshake_iter; i ++) {
		if ((ret = _talk_sender_handshake()) < 0) {
			_talk_send_abort();

			error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_TALK_HANDSHAKE_FAILED, "talk_sender(): _talk_sender_handshake()");
			error_no_return();
		}

		stage_inc(0);

		dt += ret;

		if (current->config->talk_handshake_interval)
			usleep(current->config->talk_handshake_interval * 1000);
	}

	dt /= current->config->talk_handshake_iter;

	if (net_im_connector()) {
		report_net_listener_connection_show();
	} else {
		report_net_connector_connection_show();
	}

	debug_info_talk_latency(dt);

	report_talk_latency_set(dt);
	report_talk_latency_show();

	/* Set talk context */
#ifdef UBWT_CONFIG_MULTI_THREADED
	if (worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]))
#endif
		current->talk[process_get_reverse()].count = current->config->talk_count_current;

#ifdef UBWT_CONFIG_MULTI_THREADED
	worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
#endif

	assert(current->talk[process_get_reverse()].count > 0);

	stage_reset();

	for (;;) {
		stage_set(UBWT_STAGE_STATE_RUNTIME_TALK_NEGOTIATION, bit_flag(UBWT_STAGE_CTRL_FL_NO_RESET));

		if (_talk_sender_negotiate(current->talk[process_get_reverse()].count, report_talk_latency_get()) < 0) {
			_talk_send_abort();

			error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_TALK_NEGOTIATION_FAILED, "talk_sender(): _talk_sender_negotiate()");
			error_no_return();
		}

		report_talk_count_set(current->talk[process_get_reverse()].count);
		report_talk_count_show();

		stage_set(UBWT_STAGE_STATE_RUNTIME_TALK_STREAM, bit_flag(UBWT_STAGE_CTRL_FL_NO_RESET));

		if ((ret = _talk_sender_stream(current->talk[process_get_reverse()].count))) {
			if (ret < 0) {
				_talk_send_abort();

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_TALK_STREAM_FAILED, "talk_sender(): _talk_sender_stream()");
				error_no_return();
			}

			if (!_talk_weak_stream_needs_restart())
				break;

#ifdef UBWT_CONFIG_MULTI_THREADED
			if (worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]) && _talk_weak_stream_needs_recount()) {
#endif
				if (current->config->talk_count_mul) {
					mul = current->config->talk_count_mul;
				} else {
					t = current->talk[process_get_reverse()].weak_time;

					if (!t) t = 1; /* 1us at least */

					/* We need a higher tolerance for asynchronous mode, as we cannot
					 * accuratelly predict if all the workers' streams will be considered
					 * weak at any given point in time due to link conditions, and we may end
					 * up in a very long retry loop in order to get a consistent non-weak stream.
					 * On the other hand, a very high tolerance, such as doubling the count value,
					 * may create a higher disparity between the upload and download streams, so
					 * the tolerance of the multiplier needs to sit somewhere between 1.25 and 1.75.
					 * After several tests under different link conditions on both simmetric and
					 * assimetric links, a tolerance of 1.35 for the assincronous mode and 1.20 for
					 * all the other modes seems to be a good compromise between accuracy and
					 * performance.
					 */
					mul = ((current->config->talk_stream_minimum_time * 1000000) / (double) t) * (current->config->asynchronous ? 1.35 : 1.20);

					if (mul < (current->config->asynchronous ? 1.35 : 1.20))
						mul = (current->config->asynchronous ? 1.35 : 1.20);
				}

				current->talk[process_get_reverse()].count *= mul;

#ifdef UBWT_CONFIG_MULTI_THREADED
			}

			worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
#endif

			assert(current->talk[process_get_reverse()].count <= current->config->talk_count_max);
			assert(current->talk[process_get_reverse()].count >= current->config->talk_count_default);

			stage_inc(bit_flag(UBWT_STAGE_CTRL_FL_NO_SHOW));

			continue;
		}

		break;
	}

	stage_set(UBWT_STAGE_STATE_RUNTIME_TALK_REPORT_EXCHANGE, 0);

	if (_talk_sender_report_exchange() < 0) {
		_talk_send_abort();

		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_TALK_REPORT_EXCHANGE_FAILED, "talk_sender(): _talk_sender_report_exchange()");
		error_no_return();
	}

	stage_set(UBWT_STAGE_STATE_RUNTIME_TALK_FINISH, 0);

	if (_talk_sender_finish() < 0) {
		error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_TALK_FINISH_FAILED, "talk_sender(): _talk_sender_finish()");
	}
}

void talk_receiver(void) {
	int i = 0, ret = 0;
	uint32_t count = 0, latency = 0;

	stage_set(UBWT_STAGE_STATE_RUNTIME_TALK_HANDSHAKE, 0);

#ifdef UBWT_CONFIG_MULTI_THREADED
	/* We need to sync all workers after talk_init(), and before any transmission, so all init values are syncd */
	if (current->config->asynchronous) {
		worker_barrier_wait(&current->worker_barrier_global[2]);
	} else {
		worker_barrier_wait(&current->worker_barrier_global[process_get_reverse()]);
	}
#endif

	for (i = 0; i < current->config->talk_handshake_iter; i ++) {
		if (_talk_receiver_handshake() < 0) {
			_talk_send_abort();

			error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_TALK_HANDSHAKE_FAILED, "talk_receiver(): _talk_receiver_handshake()");
			error_no_return();
		}

		stage_inc(0);
	}

	if (net_im_listener()) {
		report_net_connector_connection_show();
	} else {
		report_net_listener_connection_show();
	}

	stage_reset();

	for (;;) {
		stage_set(UBWT_STAGE_STATE_RUNTIME_TALK_NEGOTIATION, bit_flag(UBWT_STAGE_CTRL_FL_NO_RESET));

		if (_talk_receiver_negotiate(&count, &latency) < 0) {
			_talk_send_abort();

			error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_TALK_NEGOTIATION_FAILED, "talk_receiver(): _talk_receiver_negotiate()");
			error_no_return();
		}

		report_talk_count_set(count);
		report_talk_latency_set(latency);
		report_talk_count_show();
		report_talk_latency_show();

		stage_set(UBWT_STAGE_STATE_RUNTIME_TALK_STREAM, bit_flag(UBWT_STAGE_CTRL_FL_NO_RESET));

		if ((ret = _talk_receiver_stream(count))) {
			if (ret < 0) {
				_talk_send_abort();

				error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_TALK_STREAM_FAILED, "talk_receiver(): _talk_receiver_stream()");
				error_no_return();
			}

			if (!_talk_weak_stream_needs_restart())
				break;

			stage_inc(bit_flag(UBWT_STAGE_CTRL_FL_NO_SHOW));

			continue;
		}

		break;
	}

	stage_set(UBWT_STAGE_STATE_RUNTIME_TALK_REPORT_EXCHANGE, 0);

	if (_talk_receiver_report_exchange() < 0) {
		_talk_send_abort();

		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_TALK_REPORT_EXCHANGE_FAILED, "talk_receiver(): _talk_receiver_report_exchange()");
		error_no_return();
	}

	stage_set(UBWT_STAGE_STATE_RUNTIME_TALK_FINISH, 0);

	if (_talk_receiver_finish() < 0) {
		error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_TALK_FINISH_FAILED, "talk_receiver(): _talk_receiver_finish()");
	}
}

void talk_init(void) {
	memset(__talk, 0, sizeof(__talk));

	__talk[0].weak = __talk[1].weak = &__talk_weak;

	if (net_im_listener()) {
		if (net_listener_accept() < 0) {
			error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_TALK_RECEIVER_INIT, "talk_init(): net_listener_accept()");
			error_no_return();
		}

	} else {
		if (net_connector_connect() < 0) {
			error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_TALK_SENDER_INIT, "talk_init(): net_connector_connect()");
			error_no_return();
		}

	}

#ifdef UBWT_CONFIG_MULTI_THREADED
	worker_task_running_set();
#endif
}

void talk_destroy(void) {
#ifdef UBWT_CONFIG_MULTI_THREADED
	worker_task_running_unset();
#endif
}

