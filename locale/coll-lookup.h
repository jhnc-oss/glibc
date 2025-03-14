/* Copyright (C) 2000-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <stdint.h>

/* Lookup in a table of int32_t, with default value 0.  */
extern int32_t __collidx_table_lookup (const char *table, uint32_t wc)
     attribute_hidden;

/* Lookup in a table of uint32_t, with default value 0xffffffff.  */
extern uint32_t __collseq_table_lookup (const char *table, uint32_t wc)
     attribute_hidden;
