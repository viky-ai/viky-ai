/*
 *  Threads handling.
 *  Copyright (c) 2002 Pertimm bu Patrick Constant
 *  Dev : October 2002
 *  Version 1.0
*/
#include <logthr.h>
#include <sys/prctl.h>



#if (DPcSystem == DPcSystemUnix)



void *ThreadFunc(void *ptr)
{
int old_mode;
ogthread_t *IT = (ogthread_t *)ptr;
if (IT->func) {
  /** To be able to cancel the thread without delay **/
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old_mode);
  IT->ret=((*(IT->func))(IT->ptr));
  }
else IT->ret=0;
pthread_exit(NULL);
}



PUBLIC(int) OgCreateThread(IT,func,ptr)
ogthread_t *IT;
int (*func)(pr(void *));
void *ptr;
{
int retour;
char erreur[DPcSzErr];
char sys_erreur[DPcSzSysErr];
pthread_attr_t attrDetached;

IT->func = func;
IT->ptr = ptr;
IT->ret = 0;

pthread_attr_init(&attrDetached);
pthread_attr_setdetachstate(&attrDetached, PTHREAD_CREATE_DETACHED);

retour = pthread_create(&IT->hThread, &attrDetached, ThreadFunc, (void *)IT);
if (retour) {
  int nerr=OgSysErr();
  OgSysErrMes(nerr,DPcSzSysErr,sys_erreur);
  DPcSprintf(erreur,"OgCreateThread: could not pthread_create (returned %d): %s",nerr,sys_erreur);
  PcErr(-1,erreur); DPcErr;
  }
IFE(IT->ret);

DONE;
}

PUBLIC(int) OgThreadSetCurrentName(char *current_thread_name)
{
  // current_thread_name is limited to the first 16 char
  return prctl(PR_SET_NAME, current_thread_name, 0, 0, 0);
}


#else
#if (DPcSystem == DPcSystemWin32)


DWORD WINAPI ThreadFunc(LPVOID lpParam)
{
ogthread_t *IT = (ogthread_t *)lpParam;
if (IT->func) {
  IT->ret=((*(IT->func))(IT->ptr));
  }
else IT->ret=0;
DONE;
}


PUBLIC(int) OgCreateThread(IT,func,ptr)
ogthread_t *IT;
int (*func)(pr(void *));
void *ptr;
{
char sys_erreur[DPcSzSysErr];
char erreur[DPcSzErr];
IT->func = func;
IT->ptr = ptr;
IT->ret = 0;

IT->hThread = CreateThread(
    NULL,             // default security attributes
    0,                // use default stack size
    ThreadFunc,       // thread function
    (LPVOID)IT,    // argument to thread function
    0,                // use default creation flags
    &IT->dwThreadId); // returns the thread identifier

if (IT->hThread == NULL) {
  int nerr=OgSysErr();
  OgSysErrMes(nerr,DPcSzSysErr,sys_erreur);
  DPcSprintf(erreur,"OgCreateThread: could not pthread_create (returned %d): %s",nerr,sys_erreur);
  PcErr(-1,erreur); DPcErr;
  }
else {
  //CloseHandle(IT->hThread);
  }

IFE(IT->ret);
DONE;
}



#endif
#endif




