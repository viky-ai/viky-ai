/*
 *  Public header for threads and mutexes
 *  Copyright (c) 2002-2004 Pertimm by Patrick Constant
 *  Dev : October 2002, May 2003, March 2004, November, December 2004
 *  Dev : March 2005, September 2005
 *  Version 1.5
*/
#ifndef _LOGTHRALIVE_
#include <loggen.h>

#define DOgThrBanner  "ogm_thr V1.15, Copyright (c) 2002-2007 Pertimm, Inc."
#define DOgThrVersion 115

#define OgThrLabelSize  128

#if (DPcSystem == DPcSystemUnix)
#include <semaphore.h>
#include <pthread.h>

typedef struct ogthread_s {
  pthread_t hThread;
  int (*func)(pr(void *));
  void *ptr; int ret;
  } ogthread_t;

typedef struct ogmutex_s {
  char label[OgThrLabelSize];
  pthread_mutex_t mutex;
  } ogmutex_t;

typedef struct ogsem_s {
  sem_t semaphore;
  void *herr;
  } ogsem_t;

#else
#if (DPcSystem == DPcSystemWin32)

typedef struct ogthread_s {
  HANDLE hThread;
  DWORD dwThreadId;
  int (*func)(pr(void *));
  void *ptr; int ret;
  } ogthread_t;

typedef struct ogmutex_s {
  char label[OgThrLabelSize];
  CRITICAL_SECTION CriticalSection;
  } ogmutex_t;

typedef struct ogsem_s {
  HANDLE hSemaphore;
  void *herr;
  } ogsem_t;

#endif
#endif

DEFPUBLIC(int) OgCreateThread(pr_(ogthread_t *) pr_(int (*)(pr(void *))) pr(void *));
DEFPUBLIC(int) OgThreadSetCurrentName(char *current_thread_name);
DEFPUBLIC(int) OgTerminateThread(pr(ogthread_t *));

DEFPUBLIC(int) OgInitCriticalSection(ogmutex_t *mutex, char *label);
DEFPUBLIC(int) OgEnterCriticalSection(ogmutex_t *mutex);
DEFPUBLIC(int) OgEnterCriticalSectionWithTimeout(ogmutex_t *mutex, int timeout);
DEFPUBLIC(int) OgLeaveCriticalSection(ogmutex_t *mutex);
DEFPUBLIC(int) OgFlushCriticalSection(ogmutex_t *mutex);

DEFPUBLIC(int) OgSemaphoreInit(pr_(ogsem_t *) pr(int));
DEFPUBLIC(int) OgSemaphoreWait(pr(ogsem_t *));
DEFPUBLIC(int) OgSemaphorePost(pr(ogsem_t *));
DEFPUBLIC(int) OgSemaphoreFlush(pr(ogsem_t *));

DEFPUBLIC(int) OgThrMessageLog(pr_(ogmutex_t *) pr_(int) pr_(char *)
                               pr_(char *) pr_(const char *) pr(...));
DEFPUBLIC(int) OgThrErrLog(pr_(ogmutex_t *) pr_(void *) pr_(char *) pr_(int)
                           pr_(int) pr_(char *) pr(int));

#define _LOGTHRALIVE_
#endif

