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

#ifndef UBWT_USAGE_H
#define UBWT_USAGE_H

#if !defined(__GNUC__) && !defined(__clang__)
 #include <stdnoreturn.h>
#endif

#if !defined(__GNUC__) && !defined(__clang__)
_Noreturn
#endif
void usage_show(char *const *argv, int success)
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((noreturn));
#else
	;
#endif

#if !defined(__GNUC__) && !defined(__clang__)
_Noreturn
#endif
void usage_version(void)
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((noreturn));
#else
	;
#endif

void usage_check_optarg(int opt, char *optarg);

#endif

