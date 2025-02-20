/* Copyright (C) 2002-2025 Free Software Foundation, Inc.
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

#include <errno.h>
#include "pthreadP.h"
#include <shlib-compat.h>

int
__pthread_attr_setstackaddr (pthread_attr_t *attr, void *stackaddr)
{
  struct pthread_attr *iattr;

#ifdef EXTRA_PARAM_CHECKS
  EXTRA_PARAM_CHECKS;
#endif

  iattr = (struct pthread_attr *) attr;

  iattr->stackaddr = stackaddr;
  iattr->flags |= ATTR_FLAG_STACKADDR;

  return 0;
}
versioned_symbol (libc, __pthread_attr_setstackaddr,
                  pthread_attr_setstackaddr, GLIBC_2_34);
#if OTHER_SHLIB_COMPAT (libpthread, GLIBC_2_1, GLIBC_2_34)
compat_symbol (libpthread, __pthread_attr_setstackaddr,
               pthread_attr_setstackaddr, GLIBC_2_1);
#endif

link_warning (pthread_attr_setstackaddr,
              "the use of `pthread_attr_setstackaddr' is deprecated, use `pthread_attr_setstack'")
