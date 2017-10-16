/*
 *  Running a function with timeout, from wget code, Windows version
 *  Copyright (c) 2004 Pertimm bu Patrick Constant
 *  Dev : June 2004
 *  Version 1.0
*/
#include <loggen.h>
#include <assert.h>

#define HAVE_SIGNAL
#define HAVE_SIGSETJMP
#define HAVE_SIGNAL_H
#define HAVE_SETJMP_H
/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Needed for run_with_timeout. */
#undef USE_SIGNAL_TIMEOUT
#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#ifdef HAVE_SETJMP_H
# include <setjmp.h>
#endif
/* If sigsetjmp is a macro, configure won't pick it up. */
#ifdef sigsetjmp
# define HAVE_SIGSETJMP
#endif
#ifdef HAVE_SIGNAL
# ifdef HAVE_SIGSETJMP
#  define USE_SIGNAL_TIMEOUT
# endif
# ifdef HAVE_SIGBLOCK
#  define USE_SIGNAL_TIMEOUT
# endif
#endif

/* Various functions of utilitarian nature.
   Copyright (C) 1995, 1996, 1997, 1998, 2000, 2001
   Free Software Foundation, Inc.

This file is part of GNU Wget.

GNU Wget is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GNU Wget is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Wget; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

In addition, as a special exception, the Free Software Foundation
gives permission to link the code of its release of Wget with the
OpenSSL project's "OpenSSL" library (or with modified versions of it
that use the same license as the "OpenSSL" library), and distribute
the linked executables.  You must obey the GNU General Public License
in all respects for all of the code used other than "OpenSSL".  If you
modify this file, you may extend this exception to your version of the
file, but you are not obligated to do so.  If you do not wish to do
so, delete this exception statement from your version.  */


/* Implementation of run_with_timeout, a generic timeout-forcing
   routine for systems with Unix-like signal handling.  */

#ifdef USE_SIGNAL_TIMEOUT
# ifdef HAVE_SIGSETJMP
#  define SETJMP(env) sigsetjmp (env, 1)

static sigjmp_buf run_with_timeout_env;

static RETSIGTYPE
abort_run_with_timeout (int sig)
{
  assert (sig == SIGALRM);
  siglongjmp (run_with_timeout_env, -1);
}
# else /* not HAVE_SIGSETJMP */
#  define SETJMP(env) setjmp (env)

static jmp_buf run_with_timeout_env;

static RETSIGTYPE
abort_run_with_timeout (int sig)
{
  assert (sig == SIGALRM);
  /* We don't have siglongjmp to preserve the set of blocked signals;
     if we longjumped out of the handler at this point, SIGALRM would
     remain blocked.  We must unblock it manually. */
  int mask = siggetmask ();
  mask &= ~sigmask (SIGALRM);
  sigsetmask (mask);

  /* Now it's safe to longjump. */
  longjmp (run_with_timeout_env, -1);
}
# endif /* not HAVE_SIGSETJMP */

/* Arrange for SIGALRM to be delivered in TIMEOUT seconds.  This uses
   setitimer where available, alarm otherwise.

   TIMEOUT should be non-zero.  If the timeout value is so small that
   it would be rounded to zero, it is rounded to the least legal value
   instead (1us for setitimer, 1s for alarm).  That ensures that
   SIGALRM will be delivered in all cases.  */

static void
alarm_set (double timeout)
{
#ifdef ITIMER_REAL
  /* Use the modern itimer interface. */
  struct itimerval itv;
  memset (&itv, 0, sizeof (itv));
  itv.it_value.tv_sec = (long) timeout;
  itv.it_value.tv_usec = 1000000L * (timeout - (long)timeout);
  if (itv.it_value.tv_sec == 0 && itv.it_value.tv_usec == 0)
    /* Ensure that we wait for at least the minimum interval.
       Specifying zero would mean "wait forever".  */
    itv.it_value.tv_usec = 1;
  setitimer (ITIMER_REAL, &itv, NULL);
#else  /* not ITIMER_REAL */
  /* Use the old alarm() interface. */
  int secs = (int) timeout;
  if (secs == 0)
    /* Round TIMEOUTs smaller than 1 to 1, not to zero.  This is
       because alarm(0) means "never deliver the alarm", i.e. "wait
       forever", which is not what someone who specifies a 0.5s
       timeout would expect.  */
    secs = 1;
  alarm (secs);
#endif /* not ITIMER_REAL */
}

/* Cancel the alarm set with alarm_set. */

static void
alarm_cancel (void)
{
#ifdef ITIMER_REAL
  struct itimerval disable;
  memset (&disable, 0, sizeof (disable));
  setitimer (ITIMER_REAL, &disable, NULL);
#else  /* not ITIMER_REAL */
  alarm (0);
#endif /* not ITIMER_REAL */
}

/* Call FUN(ARG), but don't allow it to run for more than TIMEOUT
   seconds.  Returns non-zero if the function was interrupted with a
   timeout, zero otherwise.

   This works by setting up SIGALRM to be delivered in TIMEOUT seconds
   using setitimer() or alarm().  The timeout is enforced by
   longjumping out of the SIGALRM handler.  This has several
   advantages compared to the traditional approach of relying on
   signals causing system calls to exit with EINTR:

     * The callback function is *forcibly* interrupted after the
       timeout expires, (almost) regardless of what it was doing and
       whether it was in a syscall.  For example, a calculation that
       takes a long time is interrupted as reliably as an IO
       operation.

     * It works with both SYSV and BSD signals because it doesn't
       depend on the default setting of SA_RESTART.

     * It doesn't special handler setup beyond a simple call to
       signal().  (It does use sigsetjmp/siglongjmp, but they're
       optional.)

   The only downside is that, if FUN allocates internal resources that
   are normally freed prior to exit from the functions, they will be
   lost in case of timeout.  */

PUBLIC(int) OgRunWithTimeout(double timeout, int (*fun) (void *), void *arg, char *where)
{
  int saved_errno;

  if (timeout == 0)
    {
      fun (arg);
      return 0;
    }

  //OgMessageLog(DOgMlogInLog,where,0,"OgRunWithTimeout starting");
  signal (SIGALRM, abort_run_with_timeout);
  if (SETJMP (run_with_timeout_env) != 0)
    {
      /* Longjumped out of FUN with a timeout. */
      signal (SIGALRM, SIG_DFL);
      return 1;
    }
  alarm_set (timeout);
  fun (arg);

  //OgMessageLog(DOgMlogInLog,where,0,"OgRunWithTimeout after fun");
  /* Preserve errno in case alarm() or signal() modifies it. */
  saved_errno = errno;
  alarm_cancel ();
  signal (SIGALRM, SIG_DFL);
  errno = saved_errno;

  //OgMessageLog(DOgMlogInLog,where,0,"OgRunWithTimeout finished");
  return 0;
}

#else  /* not USE_SIGNAL_TIMEOUT */

/* A stub version of run_with_timeout that just calls FUN(ARG).  Don't
   define it under Windows, because Windows has its own version of
   run_with_timeout that uses threads.  */

PUBLIC(int) OgRunWithTimeout(double timeout, int (*fun) (void *), void *arg, char *where)
{
  fun (arg);
  return 0;
}
#endif /* not USE_SIGNAL_TIMEOUT */
