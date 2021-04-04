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
#include <inttypes.h>
#include <assert.h>
#include <math.h>

#include <arpa/inet.h>

#include "config.h"
#include "current.h"
#include "debug.h"
#include "net.h"
#include "report.h"

static uint32_t _report_bandwidth_theoretical_mbps(uint32_t bandwidth_effective) {
	/* Rough spaghetti estimate for (standard) x[G]BASE-T links... to be redesigned */
	if (bandwidth_effective < 10) return 10;
	if (bandwidth_effective < 100) return 100;
	if (bandwidth_effective < 1000) return 1000;
	if (bandwidth_effective < 2500) return 2500;
	if (bandwidth_effective < 5000) return 5000;
	if (bandwidth_effective < 10000) return 10000;
	if (bandwidth_effective < 20000) return 20000;
	if (bandwidth_effective < 40000) return 40000;

	return bandwidth_effective;
}

void report_talk_latency_set(uint32_t dt) {
	current->report.talk_latency_us = dt;
}

uint32_t report_talk_latency_get(void) {
	return current->report.talk_latency_us;
}

void report_talk_latency_show(void) {
	debug_info_report_latency(current->report.talk_latency_us);

	/* TODO: stdout progress presentation */
}

void report_talk_count_set(uint32_t count) {
	current->report.talk_count = count;
}

uint32_t report_talk_count_get(void) {
	return current->report.talk_count;
}

void report_talk_count_show(void) {
	debug_info_report_count(current->report.talk_count);

	/* TODO: stdout progress presentation */
}

void report_talk_stream_time_set(uint64_t t) {
	current->report.talk_stream_time = t;
}

uint64_t report_talk_stream_time_get(void) {
	return current->report.talk_stream_time;
}

void report_talk_stream_recv_pkts_set(uint64_t recv_pkts) {
	current->report.talk_stream_recv_pkts = recv_pkts;
}

uint64_t report_talk_stream_recv_pkts_get(void) {
	return current->report.talk_stream_recv_pkts;
}

void report_talk_stream_recv_bytes_set(uint64_t recv_bytes) {
	current->report.talk_stream_recv_bytes = recv_bytes;
}

uint64_t report_talk_stream_recv_bytes_get(void) {
	return current->report.talk_stream_recv_bytes;
}

void report_net_sender_connection_show(void) {
	char ip[256] = { 0 };
	uint16_t port = net_sockaddr_port(&current->net.sender.saddr);

	if (!net_sockaddr_ntop(&current->net.sender.saddr, ip, sizeof(ip) - 1)) {
		error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_REPORT_CONNECTION_FAILED, "report_net_sender_connection_show(): net_sockaddr_ntop()");
		return ;
	}

	debug_info_report_connection("Sender", ip, port);

	/* TODO: stdout progress presentation */
}

void report_net_receiver_connection_show(void) {
	char ip[256] = { 0 };
	uint16_t port = net_sockaddr_port(&current->net.receiver.saddr);

	if (!net_sockaddr_ntop(&current->net.receiver.saddr, ip, sizeof(ip) - 1)) {
		error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_REPORT_CONNECTION_FAILED, "report_net_receiver_connection_show(): net_sockaddr_ntop()");
		return ;
	}

	debug_info_report_connection("Receiver", ip, port);

	/* TODO: stdout progress presentation */
}

void report_marshal(void *storage, size_t len) {
	ubwt_report_t *r = (ubwt_report_t *) storage;

	assert(len >= sizeof(ubwt_report_t));

	memset(storage, 0, len);

	r->talk_latency_us = htonl(current->report.talk_latency_us);
	r->talk_count = htonl(current->report.talk_count);
	r->talk_stream_time = htonl(current->report.talk_stream_time);
	r->talk_stream_recv_pkts = htonl(current->report.talk_stream_recv_pkts);
	r->talk_stream_recv_bytes = net_htonll(current->report.talk_stream_recv_bytes);
}

void report_unmarshal(const void *storage, size_t len) {
	const ubwt_report_t *r = (ubwt_report_t *) storage;

	assert(len >= sizeof(ubwt_report_t));

	current->report.talk_latency_us = ntohl(r->talk_latency_us);
	current->report.talk_count = ntohl(r->talk_count);
	current->report.talk_stream_time = ntohl(r->talk_stream_time);
	current->report.talk_stream_recv_pkts = ntohl(r->talk_stream_recv_pkts);
	current->report.talk_stream_recv_bytes = net_ntohll(r->talk_stream_recv_bytes);
}

void report_results_show(void) {
	size_t hdr_size = 0;
	uint32_t bandwidth_theoretical_mbps = 0;
	uint64_t total_pkts = 0;
	double packet_loss = 0, fragmentation_ratio = 0;
	double bandwidth_estimated_mbps = 0, bandwidth_effective_mbps = 0;

	/* Calculate results */

	hdr_size = current->config.net_l2_hdr_size
		+ (current->net.receiver.saddr.ss_family == AF_INET6
			? current->config.net_l3_ipv6_hdr_size
			: current->config.net_l3_ipv4_hdr_size)
		+ current->config.net_l4_hdr_size;

	total_pkts = report_talk_stream_recv_bytes_get()
		/ (current->config.net_mtu < current->config.talk_payload_current_size
			? current->config.net_mtu
			: current->config.talk_payload_current_size);

	bandwidth_estimated_mbps =
		((double) ((report_talk_stream_recv_bytes_get() + (total_pkts * hdr_size)) * 8) / 1000000.0)
		/ (((double) report_talk_stream_time_get()) / 1000000.0);

	bandwidth_effective_mbps = ((double) (report_talk_stream_recv_bytes_get() * 8) / 1000000.0)
		/ (((double) report_talk_stream_time_get()) / 1000000.0);

	bandwidth_theoretical_mbps = _report_bandwidth_theoretical_mbps(bandwidth_effective_mbps);

	packet_loss = (double) ((report_talk_count_get() - report_talk_stream_recv_pkts_get()) * 100)
		/ (double) report_talk_count_get();

	fragmentation_ratio = (double) current->config.net_mtu >= current->config.talk_payload_current_size
			? (double) 1
			: (double) current->config.talk_payload_current_size / (double) current->config.net_mtu;


	/* Show results */

	fprintf(stdout, "MTU                                 : %" PRIu16 " octets\n", current->config.net_mtu);
	fprintf(stdout, "L3 Protocol                         : %s\n", current->net.receiver.saddr.ss_family == AF_INET6 ? "ipv6" : "ipv4");
	fprintf(stdout, "L4 Protocol                         : %s\n", current->config.net_l4_proto_name);
	fprintf(stdout, "Requested L4 payload size           : %" PRIu16 " octets\n", current->config.talk_payload_current_size);
	fprintf(stdout, "Estimated headers size              : %zu octets\n", hdr_size);
	fprintf(stdout, "Transmission time                   : %.4f s\n", report_talk_stream_time_get() / (double) 1000000.0);
	fprintf(stdout, "Total L4 packets expected           : %" PRIu32 "\n", report_talk_count_get());
	fprintf(stdout, "Total L4 packets transfered         : %" PRIu64 "\n", report_talk_stream_recv_pkts_get());
	fprintf(stdout, "Total L4 octets transfered          : %" PRIu64 "\n", report_talk_stream_recv_bytes_get());
	fprintf(stdout, "Estimated L3 packets transfered     : %" PRIu64 "\n", total_pkts);
	fprintf(stdout, "Estimated L3 fragmentation ratio    : %.0f:1\n", ceil(fragmentation_ratio));
	fprintf(stdout, "Estimated L2 octets transfered      : %" PRIu64 "\n", report_talk_stream_recv_bytes_get() + (total_pkts * hdr_size));
	fprintf(stdout, "Latency (L4 RTA)                    : %.3f ms\n", report_talk_latency_get() / (double) 1000.0);
	fprintf(stdout, "Effective packet loss               : %.4f (%%)\n", packet_loss);
	fprintf(stdout, "Theoretical L1 Bandwidth            : %" PRIu32 " Mbps\n", bandwidth_theoretical_mbps);
	fprintf(stdout, "Estimated L2 Bandwidth              : %.3f Mbps\n", bandwidth_estimated_mbps);
	fprintf(stdout, "Effective L4 Bandwidth              : %.3f Mbps\n", bandwidth_effective_mbps);
}

