/* Underflow signalling for narrowing functions.  Alpha version.
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

#include <fenv.h>

/* IEEE 754 determines tininess after rounding from the result rounded to
   the precision of the destination with an unbounded exponent range, but
   Alpha determines it from the delivered result.  The two differ where
   the value is tiny but reaches the smallest normal once rounded, since
   the spacing below the smallest normal is twice that of the binade the
   unbounded rounding lands in.  Alpha signals no underflow for such a
   result.  For example fdiv (-0x4p-128, 0x1.000002p+0) in FE_DOWNWARD has
   the exact quotient -0x1.fffffc0000080p-127, which rounds with an
   unbounded exponent range to -0x1.fffffep-127 and so is tiny, but is
   delivered as -0x1p-126.

   Signal underflow for that case.  RET is the narrowed result, W the
   round-to-odd value it came from and MIN_NORMAL the smallest positive
   normal value of the narrower type.  Rounding W with the exponent range
   unbounded is what decides whether underflow is due: dividing by
   MIN_NORMAL is exact and moves the value into the normal range, so the
   conversion that follows rounds the significand alone.

   The comparisons are the quiet ones, as RET is a NaN whenever an
   argument was.  */
#define CHECK_NARROW_TINY(RET, W, MIN_NORMAL)				\
  do									\
    {									\
      if (((RET) == (MIN_NORMAL) || (RET) == -(MIN_NORMAL))		\
	  && isless (W, MIN_NORMAL) && isgreater (W, -(MIN_NORMAL)))	\
	{								\
	  __typeof (RET) __scaled = (__typeof (RET)) ((W) / (MIN_NORMAL)); \
	  if (__scaled > -1 && __scaled < 1)				\
	    __feraiseexcept (FE_UNDERFLOW);				\
	}								\
    }									\
  while (0)

#endif /* math-narrow-tininess.h */
