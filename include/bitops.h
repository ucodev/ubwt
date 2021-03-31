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

#ifndef UBWT_BITOPS_H
#define UBWT_BITOPS_H

#include <stdint.h>

uint32_t bit_flag(unsigned int n);
void bit_set(volatile uint32_t *dword, unsigned int n);
void bit_clear(volatile uint32_t *dword, unsigned int n);
void bit_wipe(volatile uint32_t *dword);
void bit_toggle(volatile uint32_t *dword, unsigned int n);
unsigned int bit_test(const volatile uint32_t *dword, unsigned int n);

#endif

