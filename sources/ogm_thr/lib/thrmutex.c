/*
 *  Mutex Threads handling
 *  Copyright (c) 2003-2011 Pertimm bu Patrick Constant
 *  Dev : May 2003, September 2011
 *  Version 1.1
 */
#include <logthr.h>

#if (DPcSystem == DPcSystemUnix)

static struct timespec computeTimeout(long timeout_ms);

PUBLIC(int) OgInitCriticalSection(ogmutex_t *mutex, char *label)
{
  char erreur[DPcSzErr], sys_erreur[DPcSzErr];
  int label_length;
  IFn(mutex) DONE;
  label_length = strlen(label);
  if (label_length > OgThrLabelSize - 1) label_length = OgThrLabelSize - 1;
  memcpy(mutex->label, label, label_length);
  mutex->label[label_length] = 0;
  IFx(pthread_mutex_init(&mutex->mutex,NULL))
  {
    int nerr = OgSysErrMes(OgSysErr(), DPcSzSysErr, sys_erreur);
    sprintf(erreur, "OgInitCriticalSection on '%s': could not pthread_mutex_init: (%d) %s", mutex->label, nerr,
        sys_erreur);
    PcErr(-1, erreur);
    DPcErr;
  }
  DONE;
}

PUBLIC(int) OgEnterCriticalSection(ogmutex_t *mutex)
{
  IFn(mutex) DONE;

  int mutex_status = pthread_mutex_lock(&mutex->mutex);

  if (mutex_status != 0)
  {
    char erreur[DPcSzErr], sys_erreur[DPcSzErr];
    int nerr = OgSysErrMes(OgSysErr(), DPcSzSysErr, sys_erreur);
    sprintf(erreur, "OgEnterCriticalSection on '%s': cannot get access on mutex: (%d) %s", mutex->label, nerr,
        sys_erreur);
    PcErr(-1, erreur);
    DPcErr;
  }

  DONE;
}

PUBLIC(int) OgEnterCriticalSectionWithTimeout(ogmutex_t *mutex, int timeout_ms)
{
  IFn(mutex) DONE;

  int mutex_status = 0;

  if (timeout_ms > 0)
  {
    // add timeout_ms to current time
    struct timespec abstime = computeTimeout(timeout_ms);

    mutex_status = pthread_mutex_timedlock(&mutex->mutex, &abstime);

  }
  else
  {
    mutex_status = pthread_mutex_lock(&mutex->mutex);
  }

  if (mutex_status == ETIMEDOUT)
  {
    char erreur[DPcSzErr];
    sprintf(erreur, "OgEnterCriticalSectionWithTimeout on '%s': cannot get access on mutex,"
        " timeout after %d milliseconds", mutex->label, timeout_ms);
    PcErr(-1, erreur);
    DPcErr;
  }
  else if (mutex_status != 0)
  {
    char erreur[DPcSzErr], sys_erreur[DPcSzErr];
    int nerr = OgSysErrMes(OgSysErr(), DPcSzSysErr, sys_erreur);
    sprintf(erreur, "OgEnterCriticalSectionWithTimeout on '%s': cannot enter in mutex: (%d) %s", mutex->label, nerr,
        sys_erreur);
    PcErr(-1, erreur);
    DPcErr;
  }

  DONE;
}

PUBLIC(int) OgLeaveCriticalSection(ogmutex_t *mutex)
{
  IFn(mutex) DONE;

  int mutex_status = pthread_mutex_unlock(&mutex->mutex);

  if (mutex_status != 0)
  {
    char erreur[DPcSzErr], sys_erreur[DPcSzErr];
    int nerr = OgSysErrMes(OgSysErr(), DPcSzSysErr, sys_erreur);
    sprintf(erreur, "OgLeaveCriticalSection on '%s': cannot leave mutex: (%d) %s", mutex->label, nerr, sys_erreur);
    PcErr(-1, erreur);
    DPcErr;
  }

  DONE;
}

PUBLIC(int) OgFlushCriticalSection(ogmutex_t *mutex)
{
  char erreur[DPcSzErr], sys_erreur[DPcSzErr];
  IFn(mutex) DONE;
  IFx(pthread_mutex_destroy(&mutex->mutex))
  {
    int nerr = OgSysErrMes(OgSysErr(), DPcSzSysErr, sys_erreur);
    sprintf(erreur, "OgFlushCriticalSection on '%s': could not pthread_mutex_destroy: (%d) %s", mutex->label, nerr,
        sys_erreur);
    PcErr(-1, erreur);
    DPcErr;
  }
  DONE;
}

/**
 * Compute timeout, add timeout_ms to current time
 *
 * @param timeout_ms
 * @return timespec future timer
 */
static struct timespec computeTimeout(long timeout_ms)
{
  // get current time
  struct timeval tv[1];
  gettimeofday(tv, NULL);

  // constant conversion
  long msec_in_a_sec = 1E3;
  long nsec_in_a_usec = 1E3;
  long nsec_in_a_msec = 1E6;
  long nsec_in_a_sec = 1E9;

  // add timeout_ms to current time
  struct timespec abstime[1];
  abstime->tv_sec = tv->tv_sec + timeout_ms / msec_in_a_sec;
  abstime->tv_nsec = tv->tv_usec * nsec_in_a_usec + ((timeout_ms % msec_in_a_sec) * nsec_in_a_msec);

  // check nano sec are over 1 seconds
  if (abstime->tv_nsec >= nsec_in_a_sec)
  {
    abstime->tv_sec += abstime->tv_nsec - nsec_in_a_sec;
    abstime->tv_nsec++;
  }

  return abstime[0];
}

#else
#if (DPcSystem == DPcSystemWin32)

STATICF(int) OgEvalException(pr_(char *) pr(int));

PUBLIC(int) OgInitCriticalSection(ogmutex_t *mutex, char *label)
{
  char erreur[DPcSzErr];
  int label_length;
  IFn(mutex) DONE;
  label_length=strlen(label);
  if (label_length > OgThrLabelSize-1) label_length = OgThrLabelSize-1;
  memcpy(mutex->label,label,label_length); mutex->label[label_length]=0;
  __try
  {
    InitializeCriticalSection(&mutex->CriticalSection);
  }
  __except (OgEvalException("OgInitCriticalSection",GetExceptionCode()))
  {
    sprintf(erreur,"OgInitCriticalSection on '%s': could not InitializeCriticalSection",mutex->label);
    PcErr(-1,erreur); DPcErr;
  }
  DONE;
}

PUBLIC(int) OgEnterCriticalSection(mutex)
ogmutex_t *mutex;
{
  char erreur[DPcSzErr];
  IFn(mutex) DONE;
  __try
  {
    EnterCriticalSection(&mutex->CriticalSection);
  }
  __except (OgEvalException("OgEnterCriticalSection",GetExceptionCode()))
  {
    sprintf(erreur,"OgEnterCriticalSection: could not enter CriticalSection");
    PcErr(-1,erreur); DPcErr;
  }
  DONE;
}

PUBLIC(int) OgEnterCriticalSectionWithTimeout(mutex, timeout_ms)
ogmutex_t *mutex;
int timeout_ms;
{
  return OgEnterCriticalSection(mutex);
}

PUBLIC(int) OgLeaveCriticalSection(mutex)
ogmutex_t *mutex;
{
  IFn(mutex) DONE;
  LeaveCriticalSection(&mutex->CriticalSection);
  DONE;
}

PUBLIC(int) OgFlushCriticalSection(mutex)
ogmutex_t *mutex;
{
  IFn(mutex) DONE;
  DeleteCriticalSection(&mutex->CriticalSection);
  DONE;
}

STATICF(int) OgEvalException(function_name, n_except)
char *function_name;
int n_except;
{
  char erreur[DPcSzErr];
  sprintf(erreur, "%s received exception %x (%s)"
      , function_name ,n_except, OgExceptionToStr(n_except));
  PcErr(-1,erreur);
  return EXCEPTION_EXECUTE_HANDLER;
}

#endif
#endif

