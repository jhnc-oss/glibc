/* Definition for thread-local data handling.  NPTL/m68k version.
   Copyright (C) 2010-2025 Free Software Foundation, Inc.
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

#ifndef _TLS_H
#define _TLS_H	1

#include <dl-sysdep.h>

#ifndef __ASSEMBLER__
# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include <dl-dtv.h>
#endif /* __ASSEMBLER__ */

#ifndef __ASSEMBLER__

/* Get system call information.  */
# include <sysdep.h>

/* The TP points to the start of the thread blocks.  */
# define TLS_DTV_AT_TP	1
# define TLS_TCB_AT_TP	0

/* Get the thread descriptor definition.  */
# include <nptl/descr.h>

typedef struct
{
  dtv_t *dtv;
  void *private;
} tcbhead_t;

/* This is the size of the initial TCB.  Because our TCB is before the thread
   pointer, we don't need this.  */
# define TLS_INIT_TCB_SIZE	0

/* This is the size of the TCB.  Because our TCB is before the thread
   pointer, we don't need this.  */
# define TLS_TCB_SIZE		0

/* This is the size we need before TCB - actually, it includes the TCB.  */
# define TLS_PRE_TCB_SIZE						\
  (sizeof (struct pthread)						\
   + ((sizeof (tcbhead_t) + __alignof (struct pthread) - 1)		\
      & ~(__alignof (struct pthread) - 1)))

/* The thread pointer (TP) points to the end of the
   TCB + 0x7000, as for PowerPC and MIPS.  This implies that TCB address is
   TP - 0x7000.  As we define TLS_DTV_AT_TP we can
   assume that the pthread struct is allocated immediately ahead of the
   TCB.  This implies that the pthread_descr address is
   TP - (TLS_PRE_TCB_SIZE + 0x7000).  */
# define TLS_TCB_OFFSET	0x7000

/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
# define INSTALL_DTV(tcbp, dtvp) \
  ((tcbhead_t *) (tcbp))[-1].dtv = dtvp + 1

/* Install new dtv for current thread.  */
# define INSTALL_NEW_DTV(dtv) \
  (THREAD_DTV () = (dtv))

/* Return dtv of given thread descriptor.  */
# define GET_DTV(tcbp) \
  (((tcbhead_t *) (tcbp))[-1].dtv)

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
# define TLS_INIT_TP(tcbp)						\
  ({									\
    int _sys_result;							\
									\
    _sys_result = INTERNAL_SYSCALL_CALL (set_thread_area, 		\
				    ((void *) (tcbp)) + TLS_TCB_OFFSET); \
    !INTERNAL_SYSCALL_ERROR_P (_sys_result); })

# define TLS_DEFINE_INIT_TP(tp, pd) \
  void *tp = (void *) (pd) + TLS_TCB_OFFSET + TLS_PRE_TCB_SIZE

extern void * __m68k_read_tp (void);

/* Return the address of the dtv for the current thread.  */
# define THREAD_DTV() \
  (((tcbhead_t *) (__m68k_read_tp () - TLS_TCB_OFFSET))[-1].dtv)

/* Return the thread descriptor for the current thread.  */
# define THREAD_SELF \
  ((struct pthread *) (__m68k_read_tp () - TLS_TCB_OFFSET - TLS_PRE_TCB_SIZE))

/* Magic for libthread_db to know how to do THREAD_SELF.  */
# define DB_THREAD_SELF \
  CONST_THREAD_AREA (32, TLS_TCB_OFFSET + TLS_PRE_TCB_SIZE)

# include <tcb-access.h>

/* l_tls_offset == 0 is perfectly valid on M68K, so we have to use some
   different value to mean unset l_tls_offset.  */
# define NO_TLS_OFFSET		-1

/* Get and set the global scope generation counter in struct pthread.  */
#define THREAD_GSCOPE_FLAG_UNUSED 0
#define THREAD_GSCOPE_FLAG_USED   1
#define THREAD_GSCOPE_FLAG_WAIT   2
#define THREAD_GSCOPE_RESET_FLAG()					\
  do									\
    { int __res								\
	= atomic_exchange_release (&THREAD_SELF->header.gscope_flag,	\
			       THREAD_GSCOPE_FLAG_UNUSED);		\
      if (__res == THREAD_GSCOPE_FLAG_WAIT)				\
	lll_futex_wake (&THREAD_SELF->header.gscope_flag, 1, LLL_PRIVATE); \
    }									\
  while (0)
#define THREAD_GSCOPE_SET_FLAG()					\
  do									\
    {									\
      THREAD_SELF->header.gscope_flag = THREAD_GSCOPE_FLAG_USED;	\
      atomic_write_barrier ();						\
    }									\
  while (0)

#endif /* __ASSEMBLER__ */

#endif	/* tls.h */
