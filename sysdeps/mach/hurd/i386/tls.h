/* Definitions for thread-local data handling.  Hurd/i386 version.
   Copyright (C) 2003-2025 Free Software Foundation, Inc.
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

#ifndef _I386_TLS_H
#define _I386_TLS_H


/* Some things really need not be machine-dependent.  */
#include <sysdeps/mach/hurd/tls.h>


#ifndef __ASSEMBLER__
# include <dl-dtv.h>

/* Type of the TCB.  */
typedef struct
{
  void *tcb;			/* Points to this structure.  */
  dtv_t *dtv;			/* Vector of pointers to TLS data.  */
  thread_t self_do_not_use;	/* This thread's control port.  */
  int multiple_threads;
  uintptr_t sysinfo;
  uintptr_t stack_guard;
  uintptr_t pointer_guard;
  int gscope_flag;
  unsigned int feature_1;
  /* Reservation of some values for the TM ABI.  */
  void *__private_tm[3];
  /* GCC split stack support.  */
  void *__private_ss;
  void *__glibc_padding1;

  /* Keep these fields last, so offsets of fields above can continue being
     compatible with the i386 Linux version.  */
  mach_port_t reply_port;      /* This thread's reply port.  */
  struct hurd_sigstate *_hurd_sigstate;

  /* Used by the exception handling implementation in the dynamic loader.  */
  struct rtld_catch *rtld_catch;
} tcbhead_t;

/* GCC generates %gs:0x14 to access the stack guard.  */
_Static_assert (offsetof (tcbhead_t, stack_guard) == 0x14,
                "stack guard offset");
/* libgcc uses %gs:0x30 to access the split stack pointer.  */
_Static_assert (offsetof (tcbhead_t, __private_ss) == 0x30,
                "split stack pointer offset");

/* Return tcbhead_t from a TLS segment descriptor.  */
# define HURD_DESC_TLS(desc)						      \
  ({									      \
   (tcbhead_t *) (   (desc->low_word >> 16)				      \
                  | ((desc->high_word & 0xff) << 16)			      \
                  |  (desc->high_word & 0xff000000));			      \
  })

#endif

/* The TCB can have any size and the memory following the address the
   thread pointer points to is unspecified.  Allocate the TCB there.  */
#define TLS_TCB_AT_TP	1
#define TLS_DTV_AT_TP	0

/* Alignment requirement for TCB.

   Some processors such as Intel Atom pay a big penalty on every
   access using a segment override if that segment's base is not
   aligned to the size of a cache line.  (See Intel 64 and IA-32
   Architectures Optimization Reference Manual, section 13.3.3.3,
   "Segment Base".)  On such machines, a cache line is 64 bytes.  */
#define TCB_ALIGNMENT		64

#ifndef __ASSEMBLER__

/* Use i386-specific RPCs to arrange that %gs segment register prefix
   addresses the TCB in each thread.  */
# include <mach/i386/mach_i386.h>

# ifndef HAVE_I386_SET_GDT
#  define __i386_set_gdt(thr, sel, desc) ((void) (thr), (void) (sel), (void) (desc), MIG_BAD_ID)
# endif

# include <errno.h>
# include <assert.h>

# define HURD_TLS_DESC_DECL(desc, tcb)					      \
  struct descriptor desc =						      \
    {				/* low word: */				      \
      0xffff			/* limit 0..15 */			      \
      | (((unsigned int) (tcb)) << 16) /* base 0..15 */			      \
      ,				/* high word: */			      \
      ((((unsigned int) (tcb)) >> 16) & 0xff) /* base 16..23 */		      \
      | ((0x12 | 0x60 | 0x80) << 8) /* access = ACC_DATA_W|ACC_PL_U|ACC_P */  \
      | (0xf << 16)		/* limit 16..19 */			      \
      | ((4 | 8) << 20)		/* granularity = SZ_32|SZ_G */		      \
      | (((unsigned int) (tcb)) & 0xff000000) /* base 24..31 */		      \
    }

# define HURD_SEL_LDT(sel) (__builtin_expect ((sel) & 4, 0))

#ifndef SHARED
extern unsigned short __init1_desc;
# define __HURD_DESC_INITIAL(gs, ds) ((gs) == (ds) || (gs) == __init1_desc)
#else
# define __HURD_DESC_INITIAL(gs, ds) ((gs) == (ds))
#endif

#if !defined (SHARED) || IS_IN (rtld)
/* Return 1 if TLS is not initialized yet.  */
extern inline bool __attribute__ ((unused))
__LIBC_NO_TLS (void)
{
  unsigned short ds, gs;
  asm ("movw %%ds, %w0\n"
       "movw %%gs, %w1"
       : "=q" (ds), "=q" (gs));
  return __glibc_unlikely (__HURD_DESC_INITIAL (gs, ds));
}

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
static inline bool __attribute__ ((unused))
_hurd_tls_init (tcbhead_t *tcb, bool full)
{
  HURD_TLS_DESC_DECL (desc, tcb);
  thread_t self = __mach_thread_self ();
  bool success = true;
  extern mach_port_t __hurd_reply_port0;

  /* This field is used by TLS accesses to get our "thread pointer"
     from the TLS point of view.  */
  tcb->tcb = tcb;
  /* We always at least start the sigthread anyway.  */
  tcb->multiple_threads = 1;
  if (full)
    /* Take over the reply port we've been using.  */
    tcb->reply_port = __hurd_reply_port0;

  /* Get the first available selector.  */
  int sel = -1;
  error_t err = __i386_set_gdt (self, &sel, desc);
  if (err == MIG_BAD_ID)
    {
      /* Old kernel, use a per-thread LDT.  */
      sel = 0x27;
      err = __i386_set_ldt (self, sel, &desc, 1);
      assert_perror (err);
      if (err)
      {
	success = false;
	goto out;
      }
    }
  else if (err)
    {
      assert_perror (err); /* Separate from above with different line #. */
      success = false;
      goto out;
    }

  /* Now install the new selector.  */
  asm volatile ("mov %w0, %%gs" :: "q" (sel));
  if (full)
    /* This port is now owned by the TCB.  */
    __hurd_reply_port0 = MACH_PORT_NULL;
#ifndef SHARED
  else
    __init1_desc = sel;
#endif

out:
  __mach_port_deallocate (__mach_task_self (), self);
  return success;
}

# define TLS_INIT_TP(descr) _hurd_tls_init ((tcbhead_t *) (descr), 1)
#else /* defined (SHARED) && !IS_IN (rtld) */
# define __LIBC_NO_TLS() 0
#endif

# if __GNUC_PREREQ (6, 0)

#  define THREAD_SELF							      \
  (*(tcbhead_t * __seg_gs *) offsetof (tcbhead_t, tcb))
#  define THREAD_GETMEM(descr, member)					      \
  (*(__typeof (descr->member) __seg_gs *) offsetof (tcbhead_t, member))
#  define THREAD_GETMEM_NC(descr, member, idx)				      \
  (*(__typeof (descr->member[0]) __seg_gs *)				      \
   (offsetof (tcbhead_t, member) + (idx) * sizeof (descr->member[0])))
#  define THREAD_SETMEM(descr, member, value)				      \
  (*(__typeof (descr->member) __seg_gs *) offsetof (tcbhead_t, member) = value)
#  define THREAD_SETMEM_NC(descr, member, index, value)			      \
  (*(__typeof (descr->member[0]) __seg_gs *)				      \
   (offsetof (tcbhead_t, member) + (idx) * sizeof (descr->member[0])))

# else

/* Return the TCB address of the current thread.  */
#  define THREAD_SELF							      \
  ({ tcbhead_t *__tcb;							      \
     __asm__ ("movl %%gs:%c1,%0" : "=r" (__tcb)				      \
	      : "i" (offsetof (tcbhead_t, tcb)));			      \
     __tcb;})

/* Read member of the thread descriptor directly.  */
# define THREAD_GETMEM(descr, member) \
  ({ __typeof (descr->member) __value;					      \
     _Static_assert (sizeof (__value) == 1				      \
		     || sizeof (__value) == 4				      \
		     || sizeof (__value) == 8,				      \
		     "size of per-thread data");			      \
     if (sizeof (__value) == 1)						      \
       asm volatile ("movb %%gs:%P2,%b0"				      \
		     : "=q" (__value)					      \
		     : "0" (0), "i" (offsetof (tcbhead_t, member)));	      \
     else if (sizeof (__value) == 4)					      \
       asm volatile ("movl %%gs:%P1,%0"					      \
		     : "=r" (__value)					      \
		     : "i" (offsetof (tcbhead_t, member)));		      \
     else /* 8 */							      \
       {								      \
	 asm volatile ("movl %%gs:%P1,%%eax\n\t"			      \
		       "movl %%gs:%P2,%%edx"				      \
		       : "=A" (__value)					      \
		       : "i" (offsetof (tcbhead_t, member)),		      \
			 "i" (offsetof (tcbhead_t, member) + 4));	      \
       }								      \
     __value; })


/* Same as THREAD_GETMEM, but the member offset can be non-constant.  */
#  define THREAD_GETMEM_NC(descr, member, idx) \
  ({ __typeof (descr->member[0]) __value;				      \
     _Static_assert (sizeof (__value) == 1				      \
		     || sizeof (__value) == 4				      \
		     || sizeof (__value) == 8,				      \
		     "size of per-thread data");			      \
     if (sizeof (__value) == 1)						      \
       asm volatile ("movb %%gs:%P2(%3),%b0"				      \
		     : "=q" (__value)					      \
		     : "0" (0), "i" (offsetof (tcbhead_t, member[0])),	      \
		     "r" (idx));					      \
     else if (sizeof (__value) == 4)					      \
       asm volatile ("movl %%gs:%P1(,%2,4),%0"				      \
		     : "=r" (__value)					      \
		     : "i" (offsetof (tcbhead_t, member[0])),		      \
		       "r" (idx));					      \
     else /* 8 */							      \
       {								      \
	 asm volatile  ("movl %%gs:%P1(,%2,8),%%eax\n\t"		      \
			"movl %%gs:4+%P1(,%2,8),%%edx"			      \
			: "=&A" (__value)				      \
			: "i" (offsetof (tcbhead_t, member[0])),	      \
			  "r" (idx));					      \
       }								      \
     __value; })



/* Set member of the thread descriptor directly.  */
#  define THREAD_SETMEM(descr, member, value) \
  ({									      \
     _Static_assert (sizeof (descr->member) == 1			      \
		     || sizeof (descr->member) == 4			      \
		     || sizeof (descr->member) == 8,			      \
		     "size of per-thread data");			      \
     if (sizeof (descr->member) == 1)					      \
       asm volatile ("movb %b0,%%gs:%P1" :				      \
		     : "iq" (value),					      \
		       "i" (offsetof (tcbhead_t, member)));		      \
     else if (sizeof (descr->member) == 4)				      \
       asm volatile ("movl %0,%%gs:%P1" :				      \
		     : "ir" (value),					      \
		       "i" (offsetof (tcbhead_t, member)));		      \
     else /* 8 */							      \
       {								      \
	 asm volatile ("movl %%eax,%%gs:%P1\n\t"			      \
		       "movl %%edx,%%gs:%P2" :				      \
		       : "A" ((uint64_t) cast_to_integer (value)),	      \
			 "i" (offsetof (tcbhead_t, member)),		      \
			 "i" (offsetof (tcbhead_t, member) + 4));	      \
       }})


/* Same as THREAD_SETMEM, but the member offset can be non-constant.  */
#  define THREAD_SETMEM_NC(descr, member, idx, value) \
  ({									      \
     _Static_assert (sizeof (descr->member[0]) == 1			      \
		     || sizeof (descr->member[0]) == 4			      \
		     || sizeof (descr->member[0]) == 8,			      \
		     "size of per-thread data");			      \
     if (sizeof (descr->member[0]) == 1)				      \
       asm volatile ("movb %b0,%%gs:%P1(%2)" :				      \
		     : "iq" (value),					      \
		       "i" (offsetof (tcbhead_t, member)),		      \
		       "r" (idx));					      \
     else if (sizeof (descr->member[0]) == 4)				      \
       asm volatile ("movl %0,%%gs:%P1(,%2,4)" :			      \
		     : "ir" (value),					      \
		       "i" (offsetof (tcbhead_t, member)),		      \
		       "r" (idx));					      \
     else /* 8 */							      \
       {								      \
	 asm volatile ("movl %%eax,%%gs:%P1(,%2,8)\n\t"			      \
		       "movl %%edx,%%gs:4+%P1(,%2,8)" :			      \
		       : "A" ((uint64_t) cast_to_integer (value)),	      \
			 "i" (offsetof (tcbhead_t, member)),		      \
			 "r" (idx));					      \
       }})

# endif /* __GNUC_PREREQ (6, 0) */

/* Return the TCB address of a thread given its state.
   Note: this is expensive.  */
# define THREAD_TCB(thread, thread_state)				      \
  ({ int __sel = (thread_state)->basic.gs;				      \
     struct descriptor __desc, *___desc = &__desc;			      \
     unsigned int __count = 1;						      \
     kern_return_t __err;						      \
     if (HURD_SEL_LDT (__sel))						      \
       __err = __i386_get_ldt ((thread), __sel, 1, &___desc, &__count);	      \
     else								      \
       __err = __i386_get_gdt ((thread), __sel, &__desc);		      \
     assert_perror (__err);						      \
     assert (__count == 1);						      \
     HURD_DESC_TLS (___desc);})

/* Install new dtv for current thread.  */
# define INSTALL_NEW_DTV(dtvp) THREAD_SETMEM (THREAD_SELF, dtv, dtvp)

/* Return the address of the dtv for the current thread.  */
# define THREAD_DTV() THREAD_GETMEM (THREAD_SELF, dtv)


/* Set the stack guard field in TCB head.  */
#define THREAD_SET_STACK_GUARD(value) \
  THREAD_SETMEM (THREAD_SELF, stack_guard, value)
#define THREAD_COPY_STACK_GUARD(descr) \
  ((descr)->stack_guard							      \
   = THREAD_GETMEM (THREAD_SELF, stack_guard))

/* Set the pointer guard field in the TCB head.  */
#define THREAD_SET_POINTER_GUARD(value) \
  THREAD_SETMEM (THREAD_SELF, pointer_guard, value)
#define THREAD_COPY_POINTER_GUARD(descr) \
  ((descr)->pointer_guard						      \
   = THREAD_GETMEM (THREAD_SELF, pointer_guard))


# include <mach/machine/thread_status.h>

/* Set up TLS in the new thread of a fork child, copying from the original.  */
static inline kern_return_t __attribute__ ((unused))
_hurd_tls_fork (thread_t child, thread_t orig, struct i386_thread_state *state)
{
  /* Fetch the selector set by _hurd_tls_init.  */
  int sel;
  asm ("mov %%gs, %w0" : "=q" (sel) : "0" (0));
  if (sel == state->ds)		/* _hurd_tls_init was never called.  */
    return 0;

  struct descriptor desc, *_desc = &desc;
  error_t err;
  unsigned int count = 1;

  if (HURD_SEL_LDT (sel))
    err = __i386_get_ldt (orig, sel, 1, &_desc, &count);
  else
    err = __i386_get_gdt (orig, sel, &desc);

  assert_perror (err);
  if (err)
    return err;

  if (HURD_SEL_LDT (sel))
    err = __i386_set_ldt (child, sel, &desc, 1);
  else
    err = __i386_set_gdt (child, &sel, desc);

  state->gs = sel;
  return err;
}

static inline kern_return_t __attribute__ ((unused))
_hurd_tls_new (thread_t child, tcbhead_t *tcb)
{
  error_t err;
  /* Fetch the target thread's state.  */
  struct i386_thread_state state;
  mach_msg_type_number_t state_count = i386_THREAD_STATE_COUNT;
  err = __thread_get_state (child, i386_REGS_SEGS_STATE,
                            (thread_state_t) &state,
                            &state_count);
  if (err)
    return err;
  assert (state_count == i386_THREAD_STATE_COUNT);
  /* Fetch the selector set by _hurd_tls_init.  */
  int sel;
  asm ("mov %%gs, %w0" : "=q" (sel) : "0" (0));
  if (sel == state.ds)		/* _hurd_tls_init was never called.  */
    return 0;

  HURD_TLS_DESC_DECL (desc, tcb);

  tcb->tcb = tcb;

  if (HURD_SEL_LDT (sel))
    err = __i386_set_ldt (child, sel, &desc, 1);
  else
    err = __i386_set_gdt (child, &sel, desc);

  if (err)
    return err;

  /* Update gs to use the selector.  */
  state.gs = sel;
  return __thread_set_state (child, i386_REGS_SEGS_STATE,
                             (thread_state_t) &state,
                             state_count);
}

/* Global scope switch support.  */
# define THREAD_GSCOPE_FLAG_UNUSED 0
# define THREAD_GSCOPE_FLAG_USED   1
# define THREAD_GSCOPE_FLAG_WAIT   2

# define THREAD_GSCOPE_SET_FLAG() \
  THREAD_SETMEM (THREAD_SELF, gscope_flag, THREAD_GSCOPE_FLAG_USED)

# define THREAD_GSCOPE_RESET_FLAG() \
  ({                                                                         \
    int __flag;                                                              \
    asm volatile ("xchgl %0, %%gs:%P1"                                       \
                  : "=r" (__flag)                                            \
                  : "i" (offsetof (tcbhead_t, gscope_flag)),                 \
                    "0" (THREAD_GSCOPE_FLAG_UNUSED));                        \
    if (__flag == THREAD_GSCOPE_FLAG_WAIT)                                   \
      lll_wake (THREAD_SELF->gscope_flag, LLL_PRIVATE);                      \
  })

#endif	/* !__ASSEMBLER__ */

#endif	/* i386/tls.h */
