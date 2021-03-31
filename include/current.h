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

#ifndef UBWT_CURRENT_H
#define UBWT_CURRENT_H

#include <stdint.h>

#include "config.h"
#include "error.h"
#include "net.h"
#include "report.h"
#include "stage.h"

#ifndef UBWT_NO_PRAGMA_PACK
 #pragma pack(push)
 #pragma pack(4)
#endif
struct
#ifdef UBWT_NO_PRAGMA_PACK
__attribute__ ((packed, aligned(4)))
#endif
ubwt_current {
	struct ubwt_config config;
	struct ubwt_net net;
	uint64_t time_us;
	ubwt_stage_t stage;
	ubwt_error_t error;
	ubwt_report_t report;
};
#ifndef UBWT_NO_PRAGMA_PACK
 #pragma pack(pop)
#endif

extern struct ubwt_current *current;

void current_init(void);
void current_update(void);

#endif

