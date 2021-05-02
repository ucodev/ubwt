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
#include <inttypes.h>
#include <assert.h>
#include <math.h>

#include "config.h"
#include "current.h"
#include "debug.h"
#include "error.h"
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

#ifdef UBWT_CONFIG_MULTI_THREADED
void report_worker_combine(void) {
	unsigned int i = 0, j = 0;
	struct ubwt_current *c = NULL;

	for (c = current->next; c; c = c->next, i ++) {
		for (j = 0; j < 2; j ++) {
			current->report.results[j].collected.talk_latency_us += c->report.results[j].collected.talk_latency_us;
			current->report.results[j].collected.talk_count += c->report.results[j].collected.talk_count;
			current->report.results[j].collected.talk_stream_recv_pkts += c->report.results[j].collected.talk_stream_recv_pkts;
			current->report.results[j].collected.talk_stream_time_duration = /* Take the greater value */
				current->report.results[j].collected.talk_stream_time_duration
				> c->report.results[j].collected.talk_stream_time_duration
				? current->report.results[j].collected.talk_stream_time_duration
				: c->report.results[j].collected.talk_stream_time_duration;

			current->report.results[j].collected.talk_stream_time_start = /* Take the lesser value */
				current->report.results[j].collected.talk_stream_time_start
				> c->report.results[j].collected.talk_stream_time_start
				? c->report.results[j].collected.talk_stream_time_start
				: current->report.results[j].collected.talk_stream_time_start;
			current->report.results[j].collected.talk_stream_recv_bytes += c->report.results[j].collected.talk_stream_recv_bytes;
		}
	}

	for (j = 0; i && j < 2; j ++)
		current->report.results[j].collected.talk_latency_us /= i;
}
#endif

void report_set_straight(void) {
	current->report.result = &current->report.results[0];
}

void report_set_reverse(void) {
	current->report.result = &current->report.results[1];
}

void report_talk_latency_set(uint32_t dt) {
	current->report.result->collected.talk_latency_us = dt;
}

uint32_t report_talk_latency_get(void) {
	return current->report.result->collected.talk_latency_us;
}

void report_talk_latency_show(void) {
	debug_info_report_latency(current->report.result->collected.talk_latency_us);

	/* TODO: stdout progress presentation */
}

void report_talk_count_set(uint32_t count) {
	current->report.result->collected.talk_count = count;
}

uint32_t report_talk_count_get(void) {
	return current->report.result->collected.talk_count;
}

void report_talk_count_show(void) {
	debug_info_report_count(current->report.result->collected.talk_count);

	/* TODO: stdout progress presentation */
}

void report_talk_stream_time_start_set(uint64_t t) {
	current->report.result->collected.talk_stream_time_start = t;
}

uint64_t report_talk_stream_time_start_get(void) {
	return current->report.result->collected.talk_stream_time_start;
}

void report_talk_stream_time_duration_set(uint64_t t) {
	current->report.result->collected.talk_stream_time_duration = t;
}

uint64_t report_talk_stream_time_duration_get(void) {
	return current->report.result->collected.talk_stream_time_duration;
}

void report_talk_stream_recv_pkts_set(uint32_t recv_pkts) {
	current->report.result->collected.talk_stream_recv_pkts = recv_pkts;
}

uint32_t report_talk_stream_recv_pkts_get(void) {
	return current->report.result->collected.talk_stream_recv_pkts;
}

void report_talk_stream_recv_bytes_set(uint64_t recv_bytes) {
	current->report.result->collected.talk_stream_recv_bytes = recv_bytes;
}

uint64_t report_talk_stream_recv_bytes_get(void) {
	return current->report.result->collected.talk_stream_recv_bytes;
}

void report_net_connector_connection_show(void) {
	char ip[256] = { 0 };
	uint16_t port = net_sockaddr_port(&current->net.connector.saddr);

	if (!net_sockaddr_ntop(&current->net.connector.saddr, ip, sizeof(ip) - 1)) {
		error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_REPORT_CONNECTION_FAILED, "report_net_connector_connection_show(): net_sockaddr_ntop()");
		return ;
	}

	debug_info_report_connection("Connector", ip, port);

	/* TODO: stdout progress presentation */
}

void report_net_listener_connection_show(void) {
	char ip[256] = { 0 };
	uint16_t port = net_sockaddr_port(&current->net.listener.saddr);

	if (!net_sockaddr_ntop(&current->net.listener.saddr, ip, sizeof(ip) - 1)) {
		error_handler(UBWT_ERROR_LEVEL_WARNING, UBWT_ERROR_TYPE_REPORT_CONNECTION_FAILED, "report_net_listener_connection_show(): net_sockaddr_ntop()");
		return ;
	}

	debug_info_report_connection("Listener", ip, port);

	/* TODO: stdout progress presentation */
}

void report_marshal(void *storage, size_t len) {
	ubwt_report_collect_t *r = (ubwt_report_collect_t *) storage;

	assert(len >= sizeof(ubwt_report_collect_t));

	memset(storage, 0, len);

	r->talk_latency_us = htonl(current->report.result->collected.talk_latency_us);
	r->talk_count = htonl(current->report.result->collected.talk_count);
	r->talk_stream_time_start = net_htonll(current->report.result->collected.talk_stream_time_start);
	r->talk_stream_time_duration = net_htonll(current->report.result->collected.talk_stream_time_duration);
	r->talk_stream_recv_pkts = htonl(current->report.result->collected.talk_stream_recv_pkts);
	r->talk_stream_recv_bytes = net_htonll(current->report.result->collected.talk_stream_recv_bytes);
}

void report_unmarshal(const void *storage, size_t len) {
	const ubwt_report_collect_t *r = (ubwt_report_collect_t *) storage;

	assert(len >= sizeof(ubwt_report_collect_t));

	current->report.result->collected.talk_latency_us = ntohl(r->talk_latency_us);
	current->report.result->collected.talk_count = ntohl(r->talk_count);
	current->report.result->collected.talk_stream_time_start = net_ntohll(r->talk_stream_time_start);
	current->report.result->collected.talk_stream_time_duration = net_ntohll(r->talk_stream_time_duration);
	current->report.result->collected.talk_stream_recv_pkts = ntohl(r->talk_stream_recv_pkts);
	current->report.result->collected.talk_stream_recv_bytes = net_ntohll(r->talk_stream_recv_bytes);
}

void report_results_compute(void) {
	/* Calculate results */

	current->report.result->computed.hdr_size = current->config->net_l2_hdr_size
		+ (current->net.listener.saddr.ss_family == AF_INET6
			? current->config->net_l3_ipv6_hdr_size
			: current->config->net_l3_ipv4_hdr_size)
		+ current->config->net_l4_hdr_size;

	current->report.result->computed.total_pkts = report_talk_stream_recv_bytes_get()
		/ (current->config->net_mtu < current->config->talk_payload_current_size
			? current->config->net_mtu
			: current->config->talk_payload_current_size);

	current->report.result->computed.bandwidth_estimated_mbps =
		((double) ((report_talk_stream_recv_bytes_get() + 
		(current->report.result->computed.total_pkts *
			current->report.result->computed.hdr_size)) * 8) / 1000000.0)
		/ (((double) report_talk_stream_time_duration_get()) / 1000000.0);

	current->report.result->computed.bandwidth_effective_mbps =
		((double) (report_talk_stream_recv_bytes_get() * 8) / 1000000.0)
		/ (((double) report_talk_stream_time_duration_get()) / 1000000.0);

	current->report.result->computed.bandwidth_theoretical_mbps =
		_report_bandwidth_theoretical_mbps(current->report.result->computed.bandwidth_effective_mbps);

	current->report.result->computed.packet_loss =
		(double) ((report_talk_count_get() - report_talk_stream_recv_pkts_get()) * 100)
		/ (double) report_talk_count_get();

	current->report.result->computed.fragmentation_ratio =
			(double) current->config->net_mtu >= current->config->talk_payload_current_size
			? (double) 1
			: (double) current->config->talk_payload_current_size / (double) current->config->net_mtu;
}

void report_results_show(void) {
	/* Show results */

	puts("");

	if (current->config->report_full) {
		fprintf(stdout, "Direction                           : %s\n", process_im_receiver() ? "Download" : "Upload");
		fprintf(stdout, "MTU                                 : %" PRIu16 " octets\n", current->config->net_mtu);
		fprintf(stdout, "L3 Protocol                         : %s\n", current->net.listener.saddr.ss_family == AF_INET6 ? "ipv6" : "ipv4");
		fprintf(stdout, "L4 Protocol                         : %s\n", current->config->net_l4_proto_name);
		fprintf(stdout, "Requested L4 payload size           : %" PRIu16 " octets\n", current->config->talk_payload_current_size);
		fprintf(stdout, "Estimated headers size              : %zu octets\n", current->report.result->computed.hdr_size);
		fprintf(stdout, "Transmission time                   : %.4f s\n", report_talk_stream_time_duration_get() / (double) 1000000.0);
		fprintf(stdout, "Total L4 packets expected           : %" PRIu32 "\n", report_talk_count_get());
		fprintf(stdout, "Total L4 packets transfered         : %" PRIu32 "\n", report_talk_stream_recv_pkts_get());
		fprintf(stdout, "Total L4 octets transfered          : %" PRIu64 "\n", report_talk_stream_recv_bytes_get());
		fprintf(stdout, "Estimated L3 packets transfered     : %" PRIu64 "\n", current->report.result->computed.total_pkts);
		fprintf(stdout, "Estimated L3 fragmentation ratio    : %.0f:1\n", ceil(current->report.result->computed.fragmentation_ratio));
		fprintf(stdout, "Estimated L2 octets transfered      : %" PRIu64 "\n", report_talk_stream_recv_bytes_get() + (current->report.result->computed.total_pkts * current->report.result->computed.hdr_size));
		fprintf(stdout, "Latency (L4 RTA)                    : %.3f ms\n", report_talk_latency_get() / (double) 1000.0);
		fprintf(stdout, "Effective packet loss               : %.4f (%%)\n", current->report.result->computed.packet_loss);
		fprintf(stdout, "Theoretical L1 Bandwidth            : %" PRIu32 " Mbps\n", current->report.result->computed.bandwidth_theoretical_mbps);
		fprintf(stdout, "Estimated L2 Bandwidth              : %.3f Mbps\n", current->report.result->computed.bandwidth_estimated_mbps);
		fprintf(stdout, "Effective L4 Bandwidth              : %.3f Mbps\n", current->report.result->computed.bandwidth_effective_mbps);
	} else {
		fprintf(stdout, "%12s : %.3f ms\n", "Latency", report_talk_latency_get() / (double) 1000.0);
		fprintf(stdout, "%12s : %.3f Mbps\n", process_im_receiver() ? "Download" : "Upload", current->report.result->computed.bandwidth_estimated_mbps);
	}
}

void report_export_json_start(void) {
	FILE *fp = NULL;

	if (!current->config->report_json_file)
		return;

	if (!(fp = fopen(current->config->report_json_file, "w+"))) {
		error_handler(UBWT_ERROR_LEVEL_FATAL, UBWT_ERROR_TYPE_REPORT_JSON_FILE_FAILED, "report_export_json_start(): fopen()");

		error_no_return();
	}

	current->report.fp_export_json = fp;

	fprintf(fp, "{\n");
	fprintf(fp, "\t\"mode\": \"%s\",\n", current->config->asynchronous ? "asynchronous" : (current->config->bidirectional ? "bidirectional" : "standard"));
#ifdef UBWT_CONFIG_MULTI_THREADED
	fprintf(fp, "\t\"workers\": %u,\n", current->config->worker_count);
#else
	fprintf(fp, "\t\"workers\": 1,\n");
#endif
	fprintf(fp, "\t\"connection\": {\n");
	fprintf(fp, "\t\t\"mtu\": %" PRIu16 ",\n", current->config->net_mtu);
	fprintf(fp, "\t\t\"layer1_bandwidth_theoretical\": %" PRIu32 ",\n", current->report.result->computed.bandwidth_theoretical_mbps);
	fprintf(fp, "\t\t\"sizes\": {\n");
	fprintf(fp, "\t\t\t\"layer4_payload\": %" PRIu16 ",\n", current->config->talk_payload_current_size);
	fprintf(fp, "\t\t\t\"headers_estimated\": %zu\n", current->report.result->computed.hdr_size);
	fprintf(fp, "\t\t},\n");
	fprintf(fp, "\t\t\"protocols\": {\n");
	fprintf(fp, "\t\t\t\"layer3\": \"%s\",\n", current->net.listener.saddr.ss_family == AF_INET6 ? "ipv6" : "ipv4");
	fprintf(fp, "\t\t\t\"layer4\": \"%s\"\n", current->config->net_l4_proto_name);
	fprintf(fp, "\t\t}\n");
	fprintf(fp, "\t},\n");
}

void report_export_json_dump(int resume) {
	FILE *fp = current->report.fp_export_json;

	if (!current->config->report_json_file)
		return;

	assert(fp);

	if (resume)
		fprintf(fp, ",\n");

	fprintf(fp, "\t\"%s\": {\n", process_im_receiver() ? "download" : "upload");
	fprintf(fp, "\t\t\"octects\": {\n");
	fprintf(fp, "\t\t\t\"layer2_estimated\": %" PRIu64 ",\n", report_talk_stream_recv_bytes_get() + (current->report.result->computed.total_pkts * current->report.result->computed.hdr_size));
	fprintf(fp, "\t\t\t\"layer4_transfered\": %" PRIu64 "\n", report_talk_stream_recv_bytes_get());
	fprintf(fp, "\t\t},\n");
	fprintf(fp, "\t\t\"packets\": {\n");
	fprintf(fp, "\t\t\t\"layer3_estimated\": %" PRIu64 ",\n", current->report.result->computed.total_pkts);
	fprintf(fp, "\t\t\t\"layer4_expected\": %" PRIu32 ",\n", report_talk_count_get());
	fprintf(fp, "\t\t\t\"layer4_transfered\": %" PRIu32 "\n", report_talk_stream_recv_pkts_get());
	fprintf(fp, "\t\t},\n");
	fprintf(fp, "\t\t\"ratio\": {\n");
	fprintf(fp, "\t\t\t\"fragmentation\": %.0f,\n", ceil(current->report.result->computed.fragmentation_ratio));
	fprintf(fp, "\t\t\t\"packet_loss\": %.4f\n", current->report.result->computed.packet_loss);
	fprintf(fp, "\t\t},\n");
	fprintf(fp, "\t\t\"time\": {\n");
	fprintf(fp, "\t\t\t\"start\": %.4f,\n", report_talk_stream_time_start_get() / (double) 1000000.0);
	fprintf(fp, "\t\t\t\"duration\": %.4f,\n", report_talk_stream_time_duration_get() / (double) 1000000.0);
	fprintf(fp, "\t\t\t\"latency\": %.3f\n", report_talk_latency_get() / (double) 1000.0);
	fprintf(fp, "\t\t},\n");
	fprintf(fp, "\t\t\"bandwidth\": {\n");
	fprintf(fp, "\t\t\t\"layer2_estimated\": %.3f,\n", current->report.result->computed.bandwidth_estimated_mbps);
	fprintf(fp, "\t\t\t\"layer4_effective\": %.3f\n", current->report.result->computed.bandwidth_effective_mbps);
	fprintf(fp, "\t\t}\n");
	fprintf(fp, "\t}");

	fflush(fp);
}

void report_export_json_end(void) {
	FILE *fp = current->report.fp_export_json;

	if (!current->config->report_json_file)
		return;

	assert(fp);

	fprintf(fp, "\n}\n");
	
	fflush(fp);

	fclose(fp);
}

void report_init(void) {
	current->report.result = &current->report.results[0];
}

void report_destroy(void) {
	current->report.result = NULL;
}

