/*
 *  Mutex Threads handling
 *  Copyright (c) 2003-2011 Pertimm bu Patrick Constant
 *  Dev : May 2003, September 2011
 *  Version 1.1
*/
#include <logthr.h>



#if (DPcSystem == DPcSystemUnix)


PUBLIC(int) OgInitCriticalSection(ogmutex_t *mutex, char *label)
{
char erreur[DPcSzErr],sys_erreur[DPcSzErr];
int label_length;
IFn(mutex) DONE;
label_length=strlen(label);
if (label_length > OgThrLabelSize-1) label_length = OgThrLabelSize-1;
memcpy(mutex->label,label,label_length); mutex->label[label_length]=0;
IFx(pthread_mutex_init(&mutex->mutex,NULL)) {
  int nerr = OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
  sprintf(erreur,"OgInitCriticalSection on '%s': could not pthread_mutex_init: (%d) %s",mutex->label,nerr,sys_erreur);
  PcErr(-1,erreur); DPcErr;  
  }
DONE;
}


PUBLIC(int) OgEnterCriticalSection(mutex)
ogmutex_t *mutex;
{
IFn(mutex) DONE;
pthread_mutex_lock(&mutex->mutex);
DONE;
}


PUBLIC(int) OgLeaveCriticalSection(mutex)
ogmutex_t *mutex;
{
IFn(mutex) DONE;
pthread_mutex_unlock(&mutex->mutex);
DONE;
}


PUBLIC(int) OgFlushCriticalSection(mutex)
ogmutex_t *mutex;
{
char erreur[DPcSzErr],sys_erreur[DPcSzErr];
IFn(mutex) DONE;
IFx(pthread_mutex_destroy(&mutex->mutex)) {
  int nerr = OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
  sprintf(erreur,"OgFlushCriticalSection on '%s': could not pthread_mutex_destroy: (%d) %s",mutex->label,nerr,sys_erreur);
  PcErr(-1,erreur); DPcErr;  
  }
DONE;
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
__try {
  InitializeCriticalSection(&mutex->CriticalSection);
  }
__except (OgEvalException("OgInitCriticalSection",GetExceptionCode())) {
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
__try {
  EnterCriticalSection(&mutex->CriticalSection);
  }
__except (OgEvalException("OgEnterCriticalSection",GetExceptionCode())) {
  sprintf(erreur,"OgEnterCriticalSection: could not enter CriticalSection");
  PcErr(-1,erreur); DPcErr;  
  }
DONE;
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




