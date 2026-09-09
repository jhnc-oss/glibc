/* Test pthread_getstack_np.
   Copyright (C) 2026 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <unistd.h>
#include <support/check.h>
#include <support/namespace.h>
#include <support/support.h>
#include <support/xsignal.h>
#include <support/xthread.h>
#include <support/xunistd.h>

/* The result must match what pthread_getattr_np plus pthread_attr_getstack
   return.  */
static void
check_against_getattr_np (pthread_t thr)
{
  void *stackaddr;
  size_t stacksize;
  TEST_COMPARE (pthread_getstack_np (thr, &stackaddr, &stacksize), 0);

  pthread_attr_t attr;
  TEST_COMPARE (pthread_getattr_np (thr, &attr), 0);
  void *attr_stackaddr;
  size_t attr_stacksize;
  TEST_COMPARE (pthread_attr_getstack (&attr, &attr_stackaddr,
				       &attr_stacksize), 0);
  TEST_COMPARE (pthread_attr_destroy (&attr), 0);

  TEST_VERIFY (stackaddr == attr_stackaddr);
  TEST_COMPARE (stacksize, attr_stacksize);
}

/* A local variable of the calling thread lies within the reported bounds.  */
static void
check_contains_local (void)
{
  void *stackaddr;
  size_t stacksize;
  TEST_COMPARE (pthread_getstack_np (pthread_self (), &stackaddr,
				     &stacksize), 0);
  char here;
  TEST_VERIFY ((uintptr_t) &here >= (uintptr_t) stackaddr);
  TEST_VERIFY ((uintptr_t) &here < (uintptr_t) stackaddr + stacksize);
}

static void *
thread_func (void *arg)
{
  check_against_getattr_np (pthread_self ());
  check_contains_local ();
  return NULL;
}

static void *main_stackaddr;
static size_t main_stacksize;

/* Runs on the alternate signal stack, the reported bounds are still the ones
   of the thread stack.  */
static void
signal_handler (int sig)
{
  void *stackaddr;
  size_t stacksize;
  TEST_COMPARE (pthread_getstack_np (pthread_self (), &stackaddr, &stacksize),
		0);
  TEST_VERIFY (stackaddr == main_stackaddr);
  TEST_COMPARE (stacksize, main_stacksize);
}

/* Mount /proc so that /proc/self/maps cannot be opened, which forces the
   AT_EXECFN fallback for the initial thread.  */
static void
check_proc_fallback (void *closure)
{
  if (!support_become_root ())
    {
      puts ("info: cannot become root, skipping /proc fallback check");
      return;
    }
  if (!support_enter_mount_namespace ())
    {
      puts ("info: cannot enter mount namespace, skipping /proc fallback"
	    " check");
      return;
    }
  if (mount ("none", "/proc", "tmpfs", 0, NULL) != 0)
    {
      printf ("info: cannot mount tmpfs on /proc (%m), skipping /proc"
	      " fallback check\n");
      return;
    }

  /* The maps reader must now fail, so the fallback is exercised.  */
  TEST_VERIFY (open64 ("/proc/self/maps", O_RDONLY) < 0);

  void *stackaddr;
  size_t stacksize;
  int ret = pthread_getstack_np (pthread_self (), &stackaddr, &stacksize);

  pthread_attr_t attr;
  int gret = pthread_getattr_np (pthread_self (), &attr);
  TEST_COMPARE (ret, gret);

  if (ret == 0)
    {
      /* The fallback (downward-growing stack) reproduces the value
	 computed from the maps, captured in the parent before the fork.  */
      void *attr_stackaddr;
      size_t attr_stacksize;
      TEST_COMPARE (pthread_attr_getstack (&attr, &attr_stackaddr,
					   &attr_stacksize), 0);
      TEST_COMPARE (pthread_attr_destroy (&attr), 0);
      TEST_VERIFY (stackaddr == attr_stackaddr);
      TEST_COMPARE (stacksize, attr_stacksize);
      TEST_VERIFY (stackaddr == main_stackaddr);
      TEST_COMPARE (stacksize, main_stacksize);

      char here;
      TEST_VERIFY ((uintptr_t) &here >= (uintptr_t) stackaddr);
      TEST_VERIFY ((uintptr_t) &here < (uintptr_t) stackaddr + stacksize);
    }
  else
    {
      /* Configurations without the fallback (e.g. an upward-growing
	 stack) report the failure instead of a stack.  */
      printf ("info: no /proc fallback in this configuration (ret %d)\n",
	      ret);
      TEST_COMPARE (ret, ENOENT);
    }
}


/* Read the current [stack] vma from /proc/self/maps.  */
static bool
stack_vma (uintptr_t *lo, uintptr_t *hi)
{
  FILE *f = fopen ("/proc/self/maps", "r");
  if (f == NULL)
    return false;
  bool found = false;
  char line[512];
  while (fgets (line, sizeof line, f) != NULL)
    if (strstr (line, "[stack]") != NULL
	&& sscanf (line, "%" SCNxPTR "-%" SCNxPTR, lo, hi) == 2)
      {
	found = true;
	break;
      }
  fclose (f);
  return found;
}

static __attribute__ ((noinline)) bool
stack_grows_down_1 (uintptr_t outer)
{
  char inner;
  return (uintptr_t) &inner < outer;
}

static bool
stack_grows_down (void)
{
  char outer;
  return stack_grows_down_1 ((uintptr_t) &outer);
}

/* The kernel does not grow the stack into the guard gap above the mapping
   below it.  With a mapping planted within that distance below the current
   vma the stack can not grow at all, so the reported lowest address must be
   the current vma bottom rather than the end of the planted mapping.  This
   must be the last check, since afterwards the stack can not grow.  */
static void
check_stack_guard_gap (void)
{
  if (!stack_grows_down ())
    {
      puts ("info: stack grows up, skipping guard gap check");
      return;
    }
  uintptr_t lo, hi;
  if (!stack_vma (&lo, &hi))
    {
      puts ("info: no [stack] in /proc/self/maps, skipping guard gap check");
      return;
    }

  long pagesz = sysconf (_SC_PAGESIZE);
  uintptr_t planted = (lo - 100 * 1024) & -(uintptr_t) pagesz;
  void *m = mmap ((void *) planted, pagesz, PROT_READ | PROT_WRITE,
		  MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
  if (m == MAP_FAILED)
    {
      printf ("info: cannot map below the stack (%m), skipping guard gap"
	      " check\n");
      return;
    }
  TEST_VERIFY ((uintptr_t) m == planted);

  void *stackaddr;
  size_t stacksize;
  TEST_COMPARE (pthread_getstack_np (pthread_self (), &stackaddr, &stacksize),
		0);
  /* The vma may only have grown down since it was read.  */
  uintptr_t lo2, hi2;
  TEST_VERIFY (stack_vma (&lo2, &hi2));
  TEST_VERIFY (lo2 <= (uintptr_t) stackaddr && (uintptr_t) stackaddr <= lo);
  TEST_VERIFY ((uintptr_t) stackaddr > planted + pagesz);

  check_against_getattr_np (pthread_self ());
}

static int
do_test (void)
{
  check_against_getattr_np (pthread_self ());
  check_contains_local ();
  TEST_COMPARE (pthread_getstack_np (pthread_self (), &main_stackaddr,
				     &main_stacksize), 0);

  /* A thread with the default stack size, queried both by itself and
     by another thread.  */
  {
    pthread_t thr = xpthread_create (NULL, thread_func, NULL);
    check_against_getattr_np (thr);
    xpthread_join (thr);
  }

  /* A thread with a non-default stack size.  */
  {
    pthread_attr_t attr;
    xpthread_attr_init (&attr);
    xpthread_attr_setstacksize (&attr, 256 * 1024);
    xpthread_join (xpthread_create (&attr, thread_func, NULL));
    xpthread_attr_destroy (&attr);
  }

  /* From a signal handler running on the alternate signal stack.  */
  {
    struct support_stack altstack = support_stack_alloc (SIGSTKSZ);
    stack_t ss = { .ss_sp = altstack.stack, .ss_size = altstack.size };
    TEST_COMPARE (sigaltstack (&ss, NULL), 0);
    xsigaction (SIGUSR1,
		&(struct sigaction) {
		  .sa_handler = signal_handler,
		  .sa_flags = SA_ONSTACK },
	        NULL);
    xraise (SIGUSR1);
    support_stack_free (&altstack);
  }

  /* The initial thread fallback when /proc is not accessible.  */
  support_isolate_in_subprocess (check_proc_fallback, NULL);

  check_stack_guard_gap ();

  return 0;
}

#include <support/test-driver.c>
