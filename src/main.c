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

#include <stdlib.h>

#include "config.h"
#include "current.h"
#include "net.h"
#include "process.h"
#include "runtime.h"

static void _construct(int argc, char *const *argv) {
	current_init();

	config_init(argc, argv);

	net_init();

	report_init();

	process_init();
}

static void _destruct(void) {
	process_destroy();

	report_destroy();

	net_destroy();

	config_destroy();

	current_destroy();
}

int main(int argc, char *argv[]) {
	_construct(argc, argv);

	runtime_do();

	_destruct();

	return EXIT_SUCCESS;
}
