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
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include <sys/time.h>
#include <sys/types.h>

#include "config.h"
#include "conn.h"
#include "current.h"
#include "error.h"
#include "net.h"
#include "stage.h"

int net_connector_connect(void) {
	ubwt_conn_payload_t p1, p2;

	conn_payload_create(&p1);
	conn_payload_hton(&p1);

	if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
		if (connect(current->net.fd, (struct sockaddr *) &current->net.listener.saddr, current->net.listener.slen) < 0) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_CONNECT, "net_connector_connect(): connect()");

			return -1;
		}

#ifdef COMPILE_WIN32
		if (send(current->net.fd, (const char *) &p1, sizeof(ubwt_conn_payload_t), 0) != sizeof(ubwt_conn_payload_t)) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_CONNECT, "net_connector_connect(): send()");
#else
		if (write(current->net.fd, &p1, sizeof(ubwt_conn_payload_t)) != sizeof(ubwt_conn_payload_t)) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_CONNECT, "net_connector_connect(): write()");
#endif

			return -1;
		}

#ifdef COMPILE_WIN32
		if (recv(current->net.fd, (char *) &p2, sizeof(ubwt_conn_payload_t), MSG_WAITALL) != sizeof(ubwt_conn_payload_t)) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_CONNECT, "net_connector_connect(): recv()");
#else
		if (read(current->net.fd, &p2, sizeof(ubwt_conn_payload_t)) != sizeof(ubwt_conn_payload_t)) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_CONNECT, "net_connector_connect(): read()");
#endif
			return -1;
		}
#ifndef UBWT_CONFIG_NET_NO_UDP
	} else if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_UDP) {
 #ifdef UBWT_CONFIG_NET_UDP_CONNECT
		if (connect(current->net.fd, (struct sockaddr *) &current->net.listener.saddr, current->net.listener.slen) < 0) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_CONNECT, "net_connector_connect(): connect()");

			return -1;
		}

		if (send(current->net.fd, &p1, sizeof(ubwt_conn_payload_t), 0) < 0) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_CONNECT, "net_connector_connect(): send()");
			return -1;
		}
 #else

		if (sendto(current->net.fd, &p1, sizeof(ubwt_conn_payload_t), 0, (struct sockaddr *) &current->net.listener.saddr, current->net.listener.slen) < 0) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_CONNECT, "net_connector_connect(): sendto()");

			return -1;
		}
 #endif

		if (recvfrom(current->net.fd, &p2, sizeof(ubwt_conn_payload_t), 0, (struct sockaddr *) &current->net.listener.saddr, &current->net.listener.slen) < 0) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_CONNECT, "net_connector_connect(): recvfrom()");

			return -1;
		}
#endif
	} else {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_CONNECT, "net_connector_connect(): Unsupported L4 protocol");

		errno = EINVAL;

		return -1;
	}

	conn_payload_ntoh(&p1);
	conn_payload_ntoh(&p2);

	if (!conn_has_accept(&p2)) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_CONNECT, "net_connector_connect(): Unexpected response from the remote host");

		errno = UBWT_ERROR_MSG_UNEXPECTED;

		return -1;
	}


	if (!conn_config_match(&p1, &p2)) {
		errno = UBWT_ERROR_MSG_UNEXPECTED;

		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_NET_CONNECT, "net_connector_connect(): Configurations from listener and connector do not match");

		error_no_return();
	}

	return 0;
}

int net_listener_accept(void) {
	ubwt_conn_payload_t p1, p2;

	conn_payload_create(&p1);
	conn_payload_hton(&p1);

	if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
		current->net.connector.slen = sizeof(current->net.connector.saddr);

		if ((current->net.fd = accept(current->net.fd_listen, (struct sockaddr *) &current->net.connector.saddr, &current->net.connector.slen)) < 0) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ACCEPT, "net_listener_accept(): accept()");

			return -1;
		}

#ifdef COMPILE_WIN32
		if (recv(current->net.fd, (char *) &p2, sizeof(ubwt_conn_payload_t), MSG_WAITALL) != sizeof(ubwt_conn_payload_t)) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ACCEPT, "net_listener_accept(): recv()");
#else
		if (read(current->net.fd, &p2, sizeof(ubwt_conn_payload_t)) != sizeof(ubwt_conn_payload_t)) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ACCEPT, "net_listener_accept(): read()");
#endif
			return -1;
		}

#ifdef COMPILE_WIN32
		if (send(current->net.fd, (const char *) &p1, sizeof(ubwt_conn_payload_t), 0) != sizeof(ubwt_conn_payload_t)) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ACCEPT, "net_listener_accept(): send()");
#else
		if (write(current->net.fd, &p1, sizeof(ubwt_conn_payload_t)) != sizeof(ubwt_conn_payload_t)) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ACCEPT, "net_listener_accept(): write()");
#endif

			return -1;
		}
#ifndef UBWT_CONFIG_NET_NO_UDP
	} else if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_UDP) {
		current->net.connector.slen = sizeof(current->net.connector.saddr);

		if (recvfrom(current->net.fd, &p2, sizeof(ubwt_conn_payload_t), 0, (struct sockaddr *) &current->net.connector.saddr, &current->net.connector.slen) < 0) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ACCEPT, "net_listener_accept(): recvfrom()");

			return -1;
		}

		if (sendto(current->net.fd, &p1, sizeof(ubwt_conn_payload_t), 0, (struct sockaddr *) &current->net.connector.saddr, current->net.connector.slen) < 0) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ACCEPT, "net_listener_accept(): sendto()");

			return -1;
		}
#endif
	} else {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ACCEPT, "net_listener_accept(): Unsupported L4 protocol");

		errno = EINVAL;

		return -1;
	}

	conn_payload_ntoh(&p1);
	conn_payload_ntoh(&p2);

	if (!conn_has_connect(&p2)) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ACCEPT, "net_listener_accept(): Unexpected response from the remote host");

		errno = UBWT_ERROR_MSG_UNEXPECTED;

		return -1;
	}


	if (!conn_config_match(&p1, &p2)) {
		errno = UBWT_ERROR_MSG_UNEXPECTED;

		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_NET_ACCEPT, "net_listener_accept(): Configurations from listener and connector do not match");

		error_no_return();
	}

	return 0;
}

int net_timeout_set(sock_t fd, time_t timeout) {
#ifndef UBWT_CONFIG_NET_NO_TIMEOUT
	struct timeval tv = { 0, 0 };

	tv.tv_sec = timeout;

	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval)) < 0) {
		error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_RECV_TIMEO_SET, "net_timeout_set(): setsockopt()");
		return -1;
	}
#else
	(void) fd;
	(void) timeout;
#endif
	return 0;
}

const char *net_sockaddr_ntop(const struct sockaddr_storage *n, char *p, socklen_t len) {
	return inet_ntop(n->ss_family, n->ss_family == AF_INET6 ? (const void *) &((struct sockaddr_in6 *) n)->sin6_addr : (const void *) &((struct sockaddr_in *) n)->sin_addr, p, len);
}

uint16_t net_sockaddr_port(const struct sockaddr_storage *n) {
	return  n->ss_family == AF_INET6
		? ntohs(((struct sockaddr_in6 *) n)->sin6_port)
		: ntohs((uint16_t) ((struct sockaddr_in *) n)->sin_port);
}

ssize_t net_read_from_connector(void *buf, size_t len) {
	ssize_t count = 0, ret = 0;

	while (count < (ssize_t) len) {
		if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
#ifdef COMPILE_WIN32
			ret = recv(current->net.fd, ((char *) buf + count), len - count, MSG_WAITALL);
#else
			ret = read(current->net.fd, ((char *) buf + count), len - count);
#endif
		} else {
			ret = recv(current->net.fd, ((char *) buf + count), len - count, 0);
		}

		if (ret < 0) {
			if (errno == EINTR)
				continue;

			if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
#ifdef COMPILE_WIN32
				current->error.l_wsaerr = WSAGetLastError();

				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_RECV_FAILED, "net_read_from_connector(): recv()");
#else
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_RECV_FAILED, "net_read_from_connector(): read()");
#endif
			} else {
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_RECV_FAILED, "net_read_from_connector(): recv()");
			}

			return ret;
		}

		count += ret;
	}

	return count;
}

ssize_t net_read_from_listener(void *buf, size_t len) {
	ssize_t count = 0, ret = 0;

	while (count < (ssize_t) len) {
		if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
#ifdef COMPILE_WIN32
			ret = recv(current->net.fd, ((char *) buf + count), len - count, MSG_WAITALL);
#else
			ret = read(current->net.fd, ((char *) buf + count), len - count);
#endif
		} else {
			ret = recv(current->net.fd, ((char *) buf + count), len - count, 0);
		}

		if (ret < 0) {
			if (errno == EINTR)
				continue;

			if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
#ifdef COMPILE_WIN32
				current->error.l_wsaerr = WSAGetLastError();

				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_RECV_FAILED, "net_read_from_listener(): recv()");
#else
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_RECV_FAILED, "net_read_from_listener(): read()");
#endif
			} else {
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_RECV_FAILED, "net_read_from_listener(): recv()");
			}

			return ret;
		}

		count += ret;
	}

	return count;
}

ssize_t net_write_to_connector(const void *buf, size_t len) {
	ssize_t count = 0, ret = 0;

	while (count < (ssize_t) len) {
		if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
#ifdef COMPILE_WIN32
			ret = send(current->net.fd, ((const char *) buf) + count, len - count, 0);
#else
			ret = write(current->net.fd, ((const char *) buf) + count, len - count);
#endif
		} else {
			ret = sendto(current->net.fd, ((const char *) buf) + count, len - count, 0, (struct sockaddr *) &current->net.connector.saddr, current->net.connector.slen);
		}

		if (ret < 0) {
			if (errno == EINTR)
				continue;

#ifdef ENOBUFS
			/* If buffer space is full, wait a few microseconds before retrying */
			if (errno == ENOBUFS) {
				usleep(10);
				continue;
			}
#endif
			if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
#ifdef COMPILE_WIN32
				current->error.l_wsaerr = WSAGetLastError();

				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_SEND_FAILED, "net_write_to_connector(): send()");
#else
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_SEND_FAILED, "net_write_to_connector(): write()");
#endif
			} else {
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_SEND_FAILED, "net_write_to_connector(): sendto()");
			}

			return ret;
		}

		count += ret;
	}

	return count;
}

ssize_t net_write_to_listener(const void *buf, size_t len) {
	ssize_t count = 0, ret = 0;

	while (count < (ssize_t) len) {
		if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
#ifdef COMPILE_WIN32
			ret = send(current->net.fd, ((const char *) buf) + count, len - count, 0);
#else
			ret = write(current->net.fd, ((const char *) buf) + count, len - count);
#endif
		} else {
#ifdef UBWT_CONFIG_NET_UDP_CONNECT
			ret = send(current->net.fd, ((const char *) buf) + count, len - count, 0);
#else
			ret = sendto(current->net.fd, ((const char *) buf) + count, len - count, 0 /*MSG_CONFIRM*/, (struct sockaddr *) &current->net.listener.saddr, current->net.listener.slen);
#endif
		}

		if (ret < 0) {
			if (errno == EINTR)
				continue;

#ifdef ENOBUFS
			/* If buffer space is full, wait a few microseconds before retrying */
			if (errno == ENOBUFS) {
				usleep(10);
				continue;
			}
#endif

			if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
#ifdef COMPILE_WIN32
				current->error.l_wsaerr = WSAGetLastError();

				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_SEND_FAILED, "net_write_to_listener(): send()");
#else
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_SEND_FAILED, "net_write_to_listener(): write()");
#endif
			} else {
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_SEND_FAILED,
					"net_write_to_listener(): sendto()"
#ifdef UBWT_CONFIG_NET_UDP_CONNECT
					"send"
#else
					"sendto"
#endif
					"()");
			}

			return ret;
		}

		count += ret;
	}

	return count;
}

static int _net_connector_start(void) {
	sock_t fd = -1;
	struct addrinfo hints, *rcur = NULL, *rlist = NULL;

	memset(&hints, 0, sizeof(struct addrinfo));

	switch (current->config->net_l4_proto_value) {
		case UBWT_NET_PROTO_L4_UDP: hints.ai_socktype = SOCK_DGRAM; break;
		case UBWT_NET_PROTO_L4_TCP: hints.ai_socktype = SOCK_STREAM; break;
		default: {
			errno = EINVAL;

			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_INVALID_PROTO, "net_connector_start()");
			return -1;
		}
	}

	if ((current->error.l_eai = getaddrinfo(current->config->addr, current->config->port, &hints, &rlist))) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ADDRINFO, "net_connector_start(): getaddrinfo()");
		return -1;
	}

	for (rcur = rlist; rcur; rcur = rcur->ai_next) {
		if ((fd = socket(rcur->ai_family, rcur->ai_socktype, rcur->ai_protocol)) < 0)
			continue;

		current->net.fd = fd;
		current->net.listener.slen = rcur->ai_addrlen;
		memcpy(&current->net.listener.saddr, rcur->ai_addr, rcur->ai_addrlen);

		current->net.domain = rcur->ai_family;
		current->net.type = rcur->ai_socktype;
		current->net.protocol = rcur->ai_protocol;

		break;
	}

	if (!rcur || (fd == -1)) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ADDRINFO, "net_connector_start()");
		return -1;
	}

	freeaddrinfo(rlist);

	if (net_timeout_set(current->net.fd, current->config->net_timeout_default) < 0) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_RECV_TIMEO_SET, "net_connector_start(): net_timeout_set()");
		return -1;
	}

	return 0;
}

static int _net_listener_start(void) {
	sock_t fd = -1;
	struct addrinfo hints, *rcur = NULL, *rlist = NULL;

	memset(&hints, 0, sizeof(struct addrinfo));

	switch (current->config->net_l4_proto_value) {
		case UBWT_NET_PROTO_L4_UDP: hints.ai_socktype = SOCK_DGRAM; break;
		case UBWT_NET_PROTO_L4_TCP: hints.ai_socktype = SOCK_STREAM; break;
		default: {
			errno = EINVAL;

			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_INVALID_PROTO, "net_listener_start()");
			return -1;
		}
	}

	if ((current->error.l_eai = getaddrinfo(current->config->addr, current->config->port, &hints, &rlist))) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ADDRINFO, "net_listener_start(): getaddrinfo()");
		return -1;
	}

	for (rcur = rlist; rcur; rcur = rcur->ai_next) {
		if ((fd = socket(rcur->ai_family, rcur->ai_socktype, rcur->ai_protocol)) < 0)
			continue;

		if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
			current->net.fd_listen = fd;
		} else {
			current->net.fd = fd;
		}

		current->net.listener.slen = rcur->ai_addrlen;
		memcpy(&current->net.listener.saddr, rcur->ai_addr, rcur->ai_addrlen);

#ifdef UBWT_CONFIG_NET_REUSE_ADDRESS
		if (current->config->net_reuseaddr) {
			if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &current->config->net_reuseaddr, sizeof(current->config->net_reuseaddr)) < 0)
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_REUSEADDR_FAILED, "net_listener_start(): setsockopt()");
		}
#endif

#ifdef UBWT_CONFIG_NET_REUSE_PORT
		if (current->config->net_reuseport) {
			if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &current->config->net_reuseport, sizeof(current->config->net_reuseport)) < 0)
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_REUSEPORT_FAILED, "net_listener_start(): setsockopt()");
		}
#endif

		if (bind(fd, (struct sockaddr *) &current->net.listener.saddr, current->net.listener.slen) < 0) {
			close(fd);
			continue;
		}

		break;
	}

	if (!rcur || (fd == -1)) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ADDRINFO, "net_listener_start()");
		return -1;
	}

	freeaddrinfo(rlist);

	if (net_timeout_set(fd, current->config->net_timeout_default) < 0) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_RECV_TIMEO_SET, "net_listener_start(): net_timeout_set()");
		return -1;
	}

	if (current->config->net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
		if (listen(current->net.fd_listen, UBWT_CONFIG_NET_LISTEN_BACKLOG) < 0) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_LISTEN, "net_listener_listen(): listen()");

			return -1;
		}
	}

	return 0;
}

static void _net_listener_stop(void) {
#ifdef COMPILE_WIN32
	closesocket(current->net.fd);
	closesocket(current->net.fd_listen);
#else
	close(current->net.fd);
	close(current->net.fd_listen);
#endif
}

static void _net_connector_stop(void) {
#ifdef COMPILE_WIN32
	closesocket(current->net.fd);
#else
	close(current->net.fd);
#endif
}

void *net_buf_fill(void *buf, size_t count) {
	struct ubwt_current *c = current;

	assert(count <= c->config->net_payload_buffer_size);

	if ((c->net.buf_idx + count) > c->config->net_payload_buffer_size)
		c->net.buf_idx = 0;

	memcpy((char *) buf, ((char *) c->net.buf) + c->net.buf_idx, count);

	c->net.buf_idx += count;

	return buf;
}

void net_init(void) {
	FILE *fp = NULL;
#ifdef COMPILE_WIN32
	WORD wVer;
	WSADATA wsaData;

	wVer = MAKEWORD(2, 2);

	if (WSAStartup(wVer, &wsaData)) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_NET_INIT, "net_init(): WSAStartup()");
		error_no_return();
	}
#endif

	stage_set(UBWT_STAGE_STATE_INIT_NET, 0);

	if ((net_im_listener() ? _net_listener_start() : _net_connector_start()) < 0) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_NET_INIT, "net_init()");
		error_no_return();
	}

	if (current->config->net_payload_buffer_size) {
		if (!(fp = fopen(current->config->net_payload_buffer_file, "r"))) {
			error_handler(
				UBWT_ERROR_LEVEL_FATAL,
				UBWT_ERROR_TYPE_NET_INIT,
				"net_init(): fopen(\"%s\", \"r\")",
				current->config->net_payload_buffer_file
			);

			error_no_return();
		}

		if (!(current->net.buf = malloc(current->config->net_payload_buffer_size))) {
			error_handler(
				UBWT_ERROR_LEVEL_FATAL,
				UBWT_ERROR_TYPE_NET_INIT,
				"net_init(): malloc(%" PRIu32 ")",
				current->config->net_payload_buffer_size
			);

			error_no_return();
		}

		if (fread(current->net.buf, current->config->net_payload_buffer_size, 1, fp) != 1) {
			error_handler(
				UBWT_ERROR_LEVEL_FATAL,
				UBWT_ERROR_TYPE_NET_INIT,
				"net_init(): fread(..., %" PRIu32 ", 1, ...)",
				current->config->net_payload_buffer_size
			);

			error_no_return();
		}

		fclose(fp);
	}
}

void net_destroy(void) {
	stage_set(UBWT_STAGE_STATE_DESTROY_NET, 0);

	if (net_im_listener()) {
		_net_listener_stop();
	} else {
		_net_connector_stop();
	}

#ifdef COMPILE_WIN32
	WSACleanup();
#endif

	if (current->config->net_payload_buffer_size) {
		memset(current->net.buf, 0, current->config->net_payload_buffer_size);
		free(current->net.buf);
	}

	memset(&current->net, 0, sizeof(current->net));
}

