/* Multiple versions of __memcpy_chk
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2017-2025 Free Software Foundation, Inc.
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

/* Define multiple versions only for the definition in libc.so. */
#if IS_IN (libc) && defined SHARED
# define __memcpy_chk __redirect_memcpy_chk
# include <string.h>
# undef __memcpy_chk

# define SYMBOL_NAME memcpy_chk
# include "ifunc-memmove.h"

libc_ifunc_redirected (__redirect_memcpy_chk, __memcpy_chk,
		       IFUNC_SELECTOR ());
# ifdef SHARED
__hidden_ver1 (__memcpy_chk, __GI___memcpy_chk, __redirect_memcpy_chk)
  __attribute__ ((visibility ("hidden"))) __attribute_copy__ (__memcpy_chk);
# endif
#else
# include <debug/memcpy_chk.c>
#endif
