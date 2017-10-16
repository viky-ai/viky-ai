#include <lpcosys.h>

#if DPcSystem==DPcSystemWin32
#include <winsock2.h>
#include <windows.h>
#include <winbase.h>
#include <process.h>
#include <errno.h>
#else
#include <pthread.h> /* MUST be first */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <loggen.h>

/********************
 *
 * mutex implementation
 * 
 ********************/

/** @ingroup libpsyncAPI
 *  Use win32 unnamed mutex to allow locking hierarchy... Critical sections are one-shot . 
 **/
PUBLIC(void) OgInitMutex(THREAD_MUTEX *pm)
{
#if DPcSystem==DPcSystemWin32
  *pm = CreateMutex(NULL,FALSE,NULL);
#else 
  pthread_mutex_init (pm, NULL); // Same as  *pm = PTHREAD_MUTEX_INITIALIZER 
#endif
}

/** @ingroup libpsyncAPI
 *  Free mutex
 **/
PUBLIC(void) OgDelMutex(THREAD_MUTEX *pm)
{
#if DPcSystem==DPcSystemWin32
  CloseHandle(*pm);
#else
  pthread_mutex_destroy (pm);
#endif
}

/** @ingroup libpsyncAPI
 * Locks the specified mutex. If the mutex is already locked, the
 * calling thread blocks until the mutex becomes available. This
 * operation returns with the mutex in the locked state with the
 * calling thread as its owner.
 **/
PUBLIC(void) OgLockMutex(THREAD_MUTEX *pm)
{
#if DPcSystem==DPcSystemWin32
  WaitForSingleObject(*pm,INFINITE);
#else
  pthread_mutex_lock(pm);
#endif
}

/** @ingroup libpsyncAPI
* Try to lock the specified mutex. If the mutex is already locked,
* an error is returned. Otherwise, this operation returns with the
* mutex in the locked state with the calling thread as its owner.
*
**/
PUBLIC(int) OgTryLockMutex(THREAD_MUTEX *pm)
{  
  int rc=-1;
#if DPcSystem==DPcSystemWin32
  if (WaitForSingleObject(*pm,0) == ERROR_SUCCESS) rc=0;
#else
  if (pthread_mutex_trylock(pm)==0) rc=0;  
#endif
  return rc;
}

/** @ingroup libpsyncAPI
 * attempts to unlock the specified mutex. If there are threads
 * blocked on the mutex object when pthread_mutex_unlock() is called,
 * resulting in the mutex becoming available, the scheduling policy is
 * used to determine which thread acquires the mutex.
 **/
PUBLIC(void) OgUnlockMutex(THREAD_MUTEX *pm)
{ 
#if DPcSystem==DPcSystemWin32
  ReleaseMutex(*pm);  
#else 
  pthread_mutex_unlock(pm);
#endif
}


/** 
 *  As shown in the pthread_cond_t struct, this solution uses just one Win32 event. 
 *  This manual-reset event blocks threads in pthread_cond_wait and releases one or more threads in 
 *  pthread_cond_signal and pthread_cond_broadcast, respectively. To enhance fairness, this scheme 
 *  uses (1) a waitGenCount data member to track real signals/broadcasts and (2) 
 *  a releaseCount to control how many threads should be notified for each signal/broadcast.
 *
 *  \note get from http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
 **/
PUBLIC(void) OgInitCondMutex(THREAD_COND *mutexCondHandler)
{
#if DPcSystem==DPcSystemWin32
  mutexCondHandler->waitersCount = 0;
  mutexCondHandler->waitGenCount = 0;
  mutexCondHandler->releaseCount = 0;

  InitializeCriticalSection(&mutexCondHandler->waitersCountLock);

  // Create a manual-reset event.
  mutexCondHandler->event = CreateEvent(NULL,  // no security
			   TRUE,  // manual-reset
			   FALSE, // non-signaled initially
			   NULL); // unnamed
#else 
  pthread_cond_init(mutexCondHandler, NULL);
#endif
}

PUBLIC(void) OgDelCondMutex(THREAD_COND *mutexCondHandler) 
{
#if DPcSystem==DPcSystemWin32
  DeleteCriticalSection(&mutexCondHandler->waitersCountLock);
  CloseHandle(mutexCondHandler->event);
#else
  pthread_cond_destroy(mutexCondHandler);
#endif
}

/** 
 * This function waits for condition mutexCondHandler to be notified. When called,
 * it atomically (1) releases the associated external_mutex (which the
 * caller must hold while evaluating the condition expression) and (2)
 * goes to sleep awaiting a subsequent notification from another
 * thread (via the pthread_cond_signal or pthread_cond_broadcast
 * operations described next). The external_mutex will be locked when
 * pthread_cond_wait returns.
 * 
 * 
 *  \note get from http://www.cs.wustl.edu/~schmidt/win32-cv-1.ht
 **/																		
PUBLIC(void) OgWaitCondMutex(THREAD_COND *mutexCondHandler, THREAD_MUTEX *external_mutex)
{
#if DPcSystem==DPcSystemWin32
  int my_generation;
  int wait_done;
  int last_waiter;

  EnterCriticalSection (&mutexCondHandler->waitersCountLock);  // Avoid race conditions.
  mutexCondHandler->waitersCount++;                             // Increment count of waiters.
  my_generation = mutexCondHandler->waitGenCount;       // Store current generation in our activation record.
  LeaveCriticalSection (&mutexCondHandler->waitersCountLock);
  OgUnlockMutex(external_mutex);
  for (;;) { // Wait until the event is signaled.
    WaitForSingleObject (mutexCondHandler->event, INFINITE);
    EnterCriticalSection (&mutexCondHandler->waitersCountLock);
    // Exit the loop when the <mutexCondHandler->event> is signaled and
    // there are still waiting threads from this <wait_generation>
    // that haven't been released from this wait yet.
    wait_done = mutexCondHandler->releaseCount > 0  && mutexCondHandler->waitGenCount != my_generation;
    LeaveCriticalSection (&mutexCondHandler->waitersCountLock);
    if (wait_done) break;
  }
  OgLockMutex(external_mutex);
  EnterCriticalSection (&mutexCondHandler->waitersCountLock);
  mutexCondHandler->waitersCount--;
  mutexCondHandler->releaseCount--;
  last_waiter = mutexCondHandler->releaseCount == 0;
  LeaveCriticalSection (&mutexCondHandler->waitersCountLock);

  if (last_waiter) ResetEvent (mutexCondHandler->event); // We're the last waiter to be notified, so reset the manual event.
#else 
  pthread_cond_wait(mutexCondHandler, external_mutex);
#endif
}

/**
 * This function loops until the event HANDLE is signaled and there
 * are still threads from this ``generation'' that haven't been
 * released from the wait. The waitGenCount field is
 * incremented every time the event is signal via
 * pthread_cond_broadcast or pthread_cond_signal.  It tries to
 * eliminate the fairness problems with the SetEvents solution by not
 * responding to signal or broadcast notifications that have occurred
 * in a previous ``generation,'' i.e., before the current group of
 * threads started waiting.
 *
 * \note get from http://www.cs.wustl.edu/~schmidt/win32-cv-1.ht
 **/
PUBLIC(void) OgSignalCondMutex(THREAD_COND *mutexCondHandler)
{
#if DPcSystem==DPcSystemWin32
  EnterCriticalSection(&mutexCondHandler->waitersCountLock);
  if (mutexCondHandler->waitersCount > mutexCondHandler->releaseCount) {
    SetEvent (mutexCondHandler->event); // Signal the manual-reset event.
    mutexCondHandler->releaseCount++;
    mutexCondHandler->waitGenCount++;
  }
  LeaveCriticalSection(&mutexCondHandler->waitersCountLock);
#else
  pthread_cond_signal(mutexCondHandler);
#endif
}

/**
 * Note that we only signal the event if there are more waiters than
 * threads in the generation that is in the midst of being released.
 *
 * The following implementation of pthread_cond_broadcast notifies all
 * threads waiting on a condition variable:
 *
 * \note get from http://www.cs.wustl.edu/~schmidt/win32-cv-1.ht
 **/
PUBLIC(void) OgBroadcastCondMutex(THREAD_COND *mutexCondHandler)
{
#if DPcSystem==DPcSystemWin32
  EnterCriticalSection(&mutexCondHandler->waitersCountLock);
  if (mutexCondHandler->waitersCount > 0) {  
    SetEvent (mutexCondHandler->event);
    mutexCondHandler->releaseCount = mutexCondHandler->waitersCount;  // Release all the threads in this generation.
    mutexCondHandler->waitGenCount++;  // Start a new generation.
  }
  LeaveCriticalSection(&mutexCondHandler->waitersCountLock);
#else 
  pthread_cond_broadcast(mutexCondHandler);
#endif
}
