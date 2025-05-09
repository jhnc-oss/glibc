/* Change ownership of a file.  Linux version.
   Copyright (C) 2011-2025 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <unistd.h>
#include <fcntl.h>
#include <sysdep.h>

/* Change the owner and group of FILE.  */
int
__chown (const char *file, uid_t owner, gid_t group)
{
#ifdef __NR_chown
  return INLINE_SYSCALL_CALL (chown, file, owner, groups);
#else
  return INLINE_SYSCALL_CALL (fchownat, AT_FDCWD, file, owner, group, 0);
#endif
}
libc_hidden_def (__chown)
weak_alias (__chown, chown)
