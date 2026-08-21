/* Compute x * y + z as ternary operation.
   Copyright (C) 2010-2026 Free Software Foundation, Inc.
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

#define NO_MATH_REDIRECT
#include <math.h>
#include <fenv.h>
#include <libm-alias-float.h>
#include <math-use-builtins.h>
#include <math-narrow-tininess.h>
#include <float.h>
#include "math_config.h"

#if ! USE_FMAF_BUILTIN
/* Narrow the double result to float.  The conversion is what raises
   underflow, so architectures that do not determine tininess as IEEE 754
   describes it compensate here; elsewhere CHECK_NARROW_TINY does
   nothing and this is a plain conversion.  */
static inline float
narrow_fmaf_result (double result)
{
  float ret = (float) result;
  CHECK_NARROW_TINY (ret, result, FLT_MIN);
  return ret;
}
#endif

float
__fmaf (float x, float y, float z)
{
#if USE_FMAF_BUILTIN
  return __builtin_fmaf (x, y, z);
#else
  /* Multiplication is always exact.  */
  double xy = (double) x * (double) y;
  double result = xy + z;

  uint64_t u = asuint64 (result);
  /* If not exact or at round to even boundary, the result is correct in
     all rounding modes.  */
  if (__glibc_likely ((u & 0xfffffff) != 0))
    return narrow_fmaf_result (result);

  /* Also check if the double result appears exact when it might not be and
     thus it will not set the underflow flag if denormal.  */
  if ((u & 0x10000000) == 0
      && ((u >> MANTISSA_WIDTH) & 0x7ff) > EXPONENT_BIAS - 126)
    return narrow_fmaf_result (result);

  /* Return if result is exact in all rounding modes.  */
  if (result - xy == z && result - z == xy)
    return narrow_fmaf_result (result);

  /* This is where 'double-rouding' might return a wrong value, and thus
     needs adjusting the low-order bits in the direction of the error.  */
  double err;
  int neg = u >> 63;
  if (neg == (z > xy))
    err = xy - result + z;
  else
    err = z - result + xy;

  if (neg == (err < 0))
    u++;
  else
    u--;
  return narrow_fmaf_result (asdouble (u));
#endif /* ! USE_FMAF_BUILTIN  */
}
#ifndef __fmaf
libm_alias_float (__fma, fma)
#endif
