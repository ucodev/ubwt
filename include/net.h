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

#ifndef UBWT_NET_H
#define UBWT_NET_H

#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define net_im_sender() current->config.im_sender
#define net_im_receiver() current->config.im_receiver

typedef int sock_t;

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

#ifndef UBWT_NO_PRAGMA_PACK
 #pragma pack(push)
 #pragma pack(4)
#endif
struct
#ifdef UBWT_NO_PRAGMA_PACK
__attribute__ ((packed, aligned(4)))
#endif
ubwt_net_endpoint {
	socklen_t slen;
	struct sockaddr_storage saddr;
};

struct
#ifdef UBWT_NO_PRAGMA_PACK
__attribute__ ((packed, aligned(4)))
#endif
ubwt_net {
	sock_t fd;
	struct ubwt_net_endpoint receiver;
	struct ubwt_net_endpoint sender;
};
#ifndef UBWT_NO_PRAGMA_PACK
 #pragma pack(pop)
#endif

int net_start_sender(void);
int net_start_receiver(void);
int net_timeout_set(sock_t fd, time_t timeout);
const char *net_sockaddr_ntop(const struct sockaddr_storage *n, char *p, socklen_t len);
uint16_t net_sockaddr_port(const struct sockaddr_storage *n);
ssize_t net_read_from_sender(void *buf, size_t len);
ssize_t net_read_from_receiver(void *buf, size_t len);
ssize_t net_write_to_sender(const void *buf, size_t len);
ssize_t net_write_to_receiver(const void *buf, size_t len);
void net_init(void);

#endif
