/* Determine whether descriptor has given property.
   Copyright (C) 1996-2025 Free Software Foundation, Inc.
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
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>

int
isfdtype (int fildes, int fdtype)
{
  struct __stat64_t64 st;
  int result;

  {
    int save_error = errno;
    result = __fstat64_time64 (fildes, &st);
    __set_errno (save_error);
  }

  return result ?: (st.st_mode & S_IFMT) == (mode_t) fdtype;
}
