/*
 * Public domain.
 */

#include <math.h>
#include <errno.h>
#include <math-barriers.h>
#include <libm-alias-ldouble.h>

long double
__logbl (long double x)
{
  long double res;

  if (__glibc_unlikely (x == 0))
    {
      /* Pole error: logbl (+-0).  */
      __set_errno (ERANGE);
      return math_opt_barrier (-1.0L) / 0.0L;
    }

  asm ("fxtract\n"
       "fstp	%%st" : "=t" (res) : "0" (x));
  return res;
}

libm_alias_ldouble (__logb, logb)
