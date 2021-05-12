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

#ifndef UBWT_DEBUG_H
#define UBWT_DEBUG_H

#include <stdint.h>

#include "config.h"
#include "talk.h"

#ifdef UBWT_CONFIG_DEBUG
void debug_info_talk_op(ubwt_talk_ops_t op, const char *msg);
void debug_info_talk_latency(uint64_t dt);
void debug_info_config_show(void);
void debug_info_stage_show(void);
void debug_info_report_latency(uint32_t dt);
void debug_info_report_count(uint32_t count);
void debug_info_report_connection(const char *src, const char *ip, uint16_t port);

#else
#define debug_info_talk_op(op, msg)			while (0)
#define debug_info_talk_latency(dt)			while (0)
#define debug_info_config_show()			while (0)
#define debug_info_stage_show()				while (0)
#define debug_info_report_latency(dt)			while (0)
#define debug_info_report_count(count)			while (0)
#define debug_info_report_connection(src, ip, port)	while (0)
#endif

void debug_init(void);
void debug_update(void);
void debug_destroy(void);

#endif
