/* Underflow signalling for narrowing functions.  Generic version.
   Copyright (C) 2026 Free Software Foundation, Inc.
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

#ifndef _MATH_NARROW_TININESS_H
#define _MATH_NARROW_TININESS_H	1

/* Narrowing a round-to-odd value signals underflow by itself wherever
   tininess is determined as IEEE 754 describes it, so there is nothing
   to do here.  RET is the narrowed result, W the round-to-odd value it
   came from and MIN_NORMAL the smallest positive normal value of the
   narrower type.  */
#define CHECK_NARROW_TINY(RET, W, MIN_NORMAL) do { } while (0)

#endif /* math-narrow-tininess.h */
