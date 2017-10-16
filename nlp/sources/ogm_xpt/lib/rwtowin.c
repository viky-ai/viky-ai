/*
 *  Running a function with timeout, from wget code, Windows version
 *  Copyright (c) 2004 Pertimm bu Patrick Constant
 *  Dev : June 2004
 *  Version 1.0
*/
#define DPcInDll
#include <logxpt.h>
#include <assert.h>

/* mswindows.c -- Windows-specific support
   Copyright (C) 1995, 1996, 1997, 1998  Free Software Foundation, Inc.

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

/* run_with_timeout Windows implementation. */

 /* Stack size 0 uses default thread stack-size (reserve+commit).
  * Determined by what's in the PE header.
 */
#define THREAD_STACK_SIZE  0

struct thread_data {
   int (*fun) (void *);
   void  *arg;
   DWORD ws_error; 
};

/* The callback that runs FUN(ARG) in a separate thread.  This
   function exists for two reasons: a) to not require FUN to be
   declared WINAPI/__stdcall[1], and b) to retrieve Winsock errors,
   which are per-thread.  The latter is useful when FUN calls Winsock
   functions, which is how run_with_timeout is used in Wget.

   [1] MSVC can use __fastcall globally (cl /Gr) and on Watcom this is
   the default (wcc386 -3r).  */

static DWORD WINAPI 
thread_helper (void *arg)
{
  struct thread_data *td = (struct thread_data *) arg;

  /* Initialize Winsock error to what it was in the parent.  That way
     the subsequent call to WSAGetLastError will return the same value
     if td->fun doesn't change Winsock error state.  */
  WSASetLastError (td->ws_error);

  td->fun (td->arg);

  /* Return Winsock error to the caller, in case FUN ran Winsock
     code.  */
  td->ws_error = WSAGetLastError ();
  return 0; 
}

/* Call FUN(ARG), but don't allow it to run for more than TIMEOUT
   seconds.  Returns non-zero if the function was interrupted with a
   timeout, zero otherwise.

   This works by running FUN in a separate thread and terminating the
   thread if it doesn't finish in the specified time.  */

PUBLIC(int) OgRunWithTimeout(double seconds, int (*fun) (void *), void *arg, char *where)
{
  static HANDLE thread_hnd = NULL;
  struct thread_data thread_arg;
  DWORD  thread_id;
  int    rc = 0;

  if (seconds == 0)
    {
    blocking_fallback:
      IFE(fun(arg));
      return 0;
    }

  /* Should never happen, but test for recursivety anyway */
  assert (thread_hnd == NULL);  

  thread_arg.fun = fun;
  thread_arg.arg = arg;
  thread_arg.ws_error = WSAGetLastError ();
  thread_hnd = CreateThread (NULL, THREAD_STACK_SIZE, thread_helper,
			     &thread_arg, 0, &thread_id); 
  if (!thread_hnd)
    {
      OgMessageLog(DOgMlogInLog,where,0,"CreateThread() failed; %s\n", strerror (GetLastError ()));
      goto blocking_fallback;
    }

  if (WaitForSingleObject (thread_hnd, (DWORD)(1000 * seconds))
      == WAIT_OBJECT_0)
    {
      /* Propagate error state (which is per-thread) to this thread,
	 so the caller can inspect it.  */
      WSASetLastError (thread_arg.ws_error);
      rc = 0;
    }
  else
    {
      TerminateThread (thread_hnd, 1);
      rc = 1;
    }

  CloseHandle (thread_hnd); /* clear-up after TerminateThread() */
  thread_hnd = NULL;
  return rc;
}


