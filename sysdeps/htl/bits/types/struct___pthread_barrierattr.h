/* Thread barrier attribute type.  Generic version.
   Copyright (C) 2002-2025 Free Software Foundation, Inc.
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
   License along with the GNU C Library;  if not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _BITS_TYPES_STRUCT___PTHREAD_BARRIERATTR_H
#define _BITS_TYPES_STRUCT___PTHREAD_BARRIERATTR_H	1

enum __pthread_process_shared;

/* This structure describes the attributes of a POSIX thread barrier.
   Note that not all of them are supported on all systems.  */
struct __pthread_barrierattr
{
  enum __pthread_process_shared __pshared;
};

#endif /* bits/types/struct___pthread_barrierattr.h */
