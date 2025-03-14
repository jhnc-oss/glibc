/* Verify that MALLOC_ALIGNMENT is honored by malloc.
   Copyright (C) 2012-2025 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <malloc-size.h>
#include <support/check.h>

static void *
test (size_t s)
{
  void *p = malloc (s);

  printf ("malloc: %zu, %p: %zu\n", s, p,
	  ((uintptr_t) p) & MALLOC_ALIGN_MASK);
  return p;
}

#define ALIGNED(p) (((uintptr_t) p & MALLOC_ALIGN_MASK) == 0)

static int
do_test (void)
{
  void *p;

  p = test (2);
  TEST_VERIFY (ALIGNED (p));
  free (p);

  p = test (8);
  TEST_VERIFY (ALIGNED (p));
  free (p);

  p = test (13);
  TEST_VERIFY (ALIGNED (p));
  free (p);

  p = test (16);
  TEST_VERIFY (ALIGNED (p));
  free (p);

  p = test (23);
  TEST_VERIFY (ALIGNED (p));
  free (p);

  p = test (43);
  TEST_VERIFY (ALIGNED (p));
  free (p);

  p = test (123);
  TEST_VERIFY (ALIGNED (p));
  free (p);

  return 0;
}

#include <support/test-driver.c>
