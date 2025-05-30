/* Compute minimum of two numbers, regarding NaN as missing argument.
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
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <libm-alias-double.h>

	.text
ENTRY(__fmin)
	ucomisd	%xmm0, %xmm1
	jp	1f		// jump if unordered
	minsd	%xmm1, %xmm0
	jmp	2f

1:	ucomisd	%xmm1, %xmm1	// Is xmm1 a NaN?
	jp	3f
	// xmm0 is a NaN; xmm1 is not.  Test if xmm0 is signaling.
	movsd	%xmm0, -8(%rsp)
	testb	$0x8, -2(%rsp)
	jz	4f
	movsd	%xmm1, %xmm0	// otherwise return xmm1
	ret

3:	// xmm1 is a NaN; xmm0 may or may not be.
	ucomisd	%xmm0, %xmm0
	jp	4f
	// xmm1 is a NaN; xmm0 is not.  Test if xmm1 is signaling.
	movsd	%xmm1, -8(%rsp)
	testb	$0x8, -2(%rsp)
	jz	4f
	ret

4:	// Both arguments are NaNs, or one is a signaling NaN.
	addsd	%xmm1, %xmm0

2:	ret
END(__fmin)
libm_alias_double (__fmin, fmin)
