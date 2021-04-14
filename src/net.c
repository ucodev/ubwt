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
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include "config.h"
#include "current.h"
#include "error.h"
#include "net.h"
#include "stage.h"

int net_connector_connect(void) {
	if (current->config.net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
		if (connect(current->net.fd, (struct sockaddr *) &current->net.listener.saddr, current->net.listener.slen) < 0) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_CONNECT, "net_connector_connect(): connect()");

			return -1;
		}
	}

	return 0;
}

int net_listener_accept(void) {
	if (current->config.net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
		current->net.connector.slen = sizeof(current->net.connector.saddr);

		if ((current->net.fd = accept(current->net.fd_listen, (struct sockaddr *) &current->net.connector.saddr, &current->net.connector.slen)) < 0) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ACCEPT, "net_listener_listen(): accept()");

			return -1;
		}
	}

	return 0;
}

int net_timeout_set(sock_t fd, time_t timeout) {
	struct timeval tv = { 0, 0 };

	tv.tv_sec = timeout;

	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval)) < 0) {
		error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_RECV_TIMEO_SET, "net_timeout_set(): setsockopt()");
		return -1;
	}

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
		if (current->config.net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
			ret = read(current->net.fd, ((char *) buf + count), len - count);
		} else {
			current->net.connector.slen = sizeof(current->net.connector.saddr);

			ret = recvfrom(current->net.fd, ((char *) buf + count), len - count, 0, (struct sockaddr *) &current->net.connector.saddr, &current->net.connector.slen);
		}

		if (ret < 0) {
			if (errno == EINTR)
				continue;

			if (current->config.net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_RECV_FAILED, "net_read_from_connector(): read()");
			} else {
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_RECV_FAILED, "net_read_from_connector(): recvfrom()");
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
		if (current->config.net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
			ret = read(current->net.fd, ((char *) buf + count), len - count);
		} else {
			current->net.listener.slen = sizeof(current->net.listener.saddr);

			ret = recvfrom(current->net.fd, ((char *) buf + count), len - count, 0, (struct sockaddr *) &current->net.listener.saddr, &current->net.listener.slen);
		}

		if (ret < 0) {
			if (errno == EINTR)
				continue;

			if (current->config.net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_RECV_FAILED, "net_read_from_listener(): read()");
			} else {
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_RECV_FAILED, "net_read_from_listener(): recvfrom()");
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
		if (current->config.net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
			ret = write(current->net.fd, ((const char *) buf) + count, len - count);
		} else {
			ret = sendto(current->net.fd, ((const char *) buf) + count, len - count, 0, (struct sockaddr *) &current->net.connector.saddr, current->net.connector.slen);
		}

		if (ret < 0) {
			if (errno == EINTR)
				continue;

			if (current->config.net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_SEND_FAILED, "net_write_to_connector(): write()");
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
		if (current->config.net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
			ret = write(current->net.fd, ((const char *) buf) + count, len - count);
		} else {
			ret = sendto(current->net.fd, ((const char *) buf) + count, len - count, 0 /*MSG_CONFIRM*/, (struct sockaddr *) &current->net.listener.saddr, current->net.listener.slen);
		}

		if (ret < 0) {
			if (errno == EINTR)
				continue;

			if (current->config.net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_SEND_FAILED, "net_write_to_listener(): write()");
			} else {
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_SEND_FAILED, "net_write_to_listener(): sendto()");
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

	switch (current->config.net_l4_proto_value) {
		case UBWT_NET_PROTO_L4_UDP: hints.ai_socktype = SOCK_DGRAM; break;
		case UBWT_NET_PROTO_L4_TCP: hints.ai_socktype = SOCK_STREAM; break;
		default: {
			errno = EINVAL;

			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_INVALID_PROTO, "net_connector_start()");
			return -1;
		}
	}

	if ((current->error.l_eai = getaddrinfo(current->config.addr, current->config.port, &hints, &rlist))) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ADDRINFO, "net_connector_start(): getaddrinfo()");
		return -1;
	}

	for (rcur = rlist; rcur; rcur = rcur->ai_next) {
		if ((fd = socket(rcur->ai_family, rcur->ai_socktype, rcur->ai_protocol)) < 0)
			continue;

		current->net.fd = fd;
		current->net.listener.slen = rcur->ai_addrlen;
		memcpy(&current->net.listener.saddr, rcur->ai_addr, rcur->ai_addrlen);

		break;
	}

	if (!rcur || (fd == -1)) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ADDRINFO, "net_connector_start()");
		return -1;
	}

	freeaddrinfo(rlist);

	if (net_timeout_set(current->net.fd, current->config.net_timeout_default) < 0) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_RECV_TIMEO_SET, "net_connector_start(): net_timeout_set()");
		return -1;
	}

	return 0;
}

static int _net_listener_start(void) {
	sock_t fd = -1;
	struct addrinfo hints, *rcur = NULL, *rlist = NULL;

	memset(&hints, 0, sizeof(struct addrinfo));

	switch (current->config.net_l4_proto_value) {
		case UBWT_NET_PROTO_L4_UDP: hints.ai_socktype = SOCK_DGRAM; break;
		case UBWT_NET_PROTO_L4_TCP: hints.ai_socktype = SOCK_STREAM; break;
		default: {
			errno = EINVAL;

			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_INVALID_PROTO, "net_listener_start()");
			return -1;
		}
	}

	if ((current->error.l_eai = getaddrinfo(current->config.addr, current->config.port, &hints, &rlist))) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_ADDRINFO, "net_listener_start(): getaddrinfo()");
		return -1;
	}

	for (rcur = rlist; rcur; rcur = rcur->ai_next) {
		if ((fd = socket(rcur->ai_family, rcur->ai_socktype, rcur->ai_protocol)) < 0)
			continue;

		if (current->config.net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
			current->net.fd_listen = fd;
		} else {
			current->net.fd = fd;
		}

		current->net.listener.slen = rcur->ai_addrlen;
		memcpy(&current->net.listener.saddr, rcur->ai_addr, rcur->ai_addrlen);

		if (current->config.net_reuseaddr) {
			if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &current->config.net_reuseaddr, sizeof(current->config.net_reuseaddr)) < 0)
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_REUSEADDR_FAILED, "net_listener_start(): setsockopt()");
		}

		if (current->config.net_reuseport) {
			if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &current->config.net_reuseport, sizeof(current->config.net_reuseport)) < 0)
				error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_NET_REUSEPORT_FAILED, "net_listener_start(): setsockopt()");
		}

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

	if (net_timeout_set(fd, current->config.net_timeout_default) < 0) {
		error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_RECV_TIMEO_SET, "net_listener_start(): net_timeout_set()");
		return -1;
	}

	if (current->config.net_l4_proto_value == UBWT_NET_PROTO_L4_TCP) {
		if (listen(current->net.fd_listen, UBWT_CONFIG_NET_LISTEN_BACKLOG) < 0) {
			error_handler(UBWT_ERROR_LEVEL_CRITICAL, UBWT_ERROR_TYPE_NET_LISTEN, "net_listener_listen(): listen()");

			return -1;
		}
	}

	return 0;
}

static void _net_listener_stop(void) {
	close(current->net.fd);
	close(current->net.fd_listen);
}

static void _net_connector_stop(void) {
	close(current->net.fd);
}

void net_init(void) {
	stage_set(UBWT_STAGE_STATE_INIT_NET, 0);

	if ((net_im_listener() ? _net_listener_start() : _net_connector_start()) < 0) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_NET_INIT, "net_init()");
		error_no_return();
	}
}

void net_destroy(void) {
	stage_set(UBWT_STAGE_STATE_DESTROY_NET, 0);

	if (net_im_listener()) {
		_net_listener_stop();
	} else {
		_net_connector_stop();
	}

	memset(&current->net, 0, sizeof(current->net));
}

