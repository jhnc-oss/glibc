#include <math_ldbl_opt.h>
#include <libm-alias-ldouble.h>
#if IS_IN (libc) && defined SHARED
# undef libm_alias_ldouble
# define libm_alias_ldouble(from, to)
#endif
#include <sysdeps/ieee754/ldbl-128/s_frexpl.c>
#if IS_IN (libc) && defined SHARED
long_double_symbol (libc, __frexpl, frexpl);
#endif
