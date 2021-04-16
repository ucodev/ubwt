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

#ifndef UBWT_NET_H
#define UBWT_NET_H

#include <string.h>
#include <stdint.h>

#include <sys/types.h>

#ifdef COMPILE_WIN32
 #include <winsock2.h>
 #include <windows.h>
 #include <ws2tcpip.h>

 typedef SOCKET sock_t
#else
 #include <sys/socket.h>

 #include <arpa/inet.h>
 #include <netdb.h>

 #include <netinet/in.h>

 typedef int sock_t;
#endif

#define net_im_connector() current->config.im_connector
#define net_im_listener() current->config.im_listener

#if defined(__clang__) && !defined(__cplusplus)
#define net_htonll(hostlonglong) 	(*(unsigned char *) (unsigned int [1]) { 1 }) ? ((uint64_t) htonl(((uint32_t *) &((uint64_t [1]) { (hostlonglong) }[0]))[0]) << 32) | htonl(((uint32_t *) &((uint64_t [1]) { (hostlonglong) }[0]))[1]) : (hostlonglong)
#else
static inline uint64_t net_htonll(uint64_t hostlonglong) {
	uint32_t *ptr = (uint32_t *) (void *) &hostlonglong;
	uint32_t hi = 0, lo = 0;
	unsigned int i = 1;

	if (*(unsigned char *) &i) {
		lo = htonl(ptr[1]);
		hi = htonl(ptr[0]);
		memcpy(ptr, &lo, 4);
		memcpy(&ptr[1], &hi, 4);
	}

	return hostlonglong;
}
#endif

#define net_ntohll(netlonglong)	(net_htonll(netlonglong))

enum UBWT_NET_PROTO_L4_VALUES {
	UBWT_NET_PROTO_L4_TCP = 1,
	UBWT_NET_PROTO_L4_UDP
};

struct ubwt_net_endpoint {
	socklen_t slen;
	struct sockaddr_storage saddr;
};

struct ubwt_net {
	sock_t fd;
	sock_t fd_listen;
	struct ubwt_net_endpoint listener;
	struct ubwt_net_endpoint connector;
};

int net_connector_connect(void);
int net_listener_accept(void);
int net_timeout_set(sock_t fd, time_t timeout);
const char *net_sockaddr_ntop(const struct sockaddr_storage *n, char *p, socklen_t len);
uint16_t net_sockaddr_port(const struct sockaddr_storage *n);
ssize_t net_read_from_connector(void *buf, size_t len);
ssize_t net_read_from_listener(void *buf, size_t len);
ssize_t net_write_to_connector(const void *buf, size_t len);
ssize_t net_write_to_listener(const void *buf, size_t len);
void net_init(void);
void net_destroy(void);

#endif
