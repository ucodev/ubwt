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

#ifndef UBWT_PROCESS_H
#define UBWT_PROCESS_H

#include <stdint.h>

#define process_im_sender()	current->process.im_sender
#define process_im_receiver()	current->process.im_receiver
#define process_get_reverse()	current->process.reverse

struct ubwt_process {
	uint8_t im_sender;
	uint8_t im_receiver;
	uint8_t reverse;
	uint8_t __reserved;
};

void process_set_im_sender(void);
void process_set_im_receiver(void);
void process_set_reverse(void);
void process_set_straight(void);
void process_init(void);
void process_run(int reverse_first);
void process_report(void);
void process_destroy(void);

#endif

