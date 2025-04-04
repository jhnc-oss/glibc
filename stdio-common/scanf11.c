/* Copyright (C) 2001-2025 Free Software Foundation, Inc.
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

#include <ctype.h>
#include <stdio.h>

int
main (int argc, char *argv[])
{
  int exc = 0;
  int retc;
  float f;
  int d;
  char str[] = "x 1";
  int c;

  for (c = 1; c < 127; ++c)
    if (! isdigit (c) && ! isspace (c))
      {
	str[0] = c;
	retc = sscanf (str, "%e %d", &f, &d);
	if (retc != 0)
	  {
	    printf ("sscanf (\"%s\", \"%%e %%d\", ...) == %d, not 0\n",
		    str, retc);
	    exc = 1;
	  }
      }

  return exc;
}
