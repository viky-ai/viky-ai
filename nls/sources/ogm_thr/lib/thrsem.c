/*
 *  Semaphores handling
 *  Copyright (c) 2007 Pertimm bu Patrick Constant
 *  Dev : February 2007
 *  Version 1.0
*/
#include <logthr.h>



#if (DPcSystem == DPcSystemUnix)


PUBLIC(int) OgSemaphoreInit(sem,value)
ogsem_t *sem; int value;
{
char sys_erreur[DOgErrorSize];
char erreur[DOgErrorSize];

IFn(sem) DONE;

IF(sem_init(&sem->semaphore,0,value)) {
  int nerr = OgSysErr();
  OgSysErrMes(nerr,DOgErrorSize-100,sys_erreur);
  sprintf(erreur,"OgSemaphoreInit: sem_init error %d: %s",nerr,sys_erreur);
  OgErr(sem->herr,erreur); DPcErr;
  }

DONE;
}



PUBLIC(int) OgSemaphoreWait(sem)
ogsem_t *sem;
{
IFn(sem) DONE;
sem_wait(&sem->semaphore);
DONE;
}



PUBLIC(int) OgSemaphorePost(sem)
ogsem_t *sem;
{
IFn(sem) DONE;
sem_post(&sem->semaphore);
DONE;
}



PUBLIC(int) OgSemaphoreFlush(sem)
ogsem_t *sem;
{
IFn(sem) DONE;
sem_destroy(&sem->semaphore);
DONE;
}



#else
#if (DPcSystem == DPcSystemWin32)

PUBLIC(int) OgSemaphoreInit(sem,value)
ogsem_t *sem; int value;
{
char sys_erreur[DOgErrorSize];
char erreur[DOgErrorSize];

IFn(sem) DONE;

sem->hSemaphore = CreateSemaphore( 
    NULL,   // default security attributes
    value,   // initial count
    0x7fffffff,   // maximum count
    NULL);  // unnamed semaphore
IFn(sem->hSemaphore) {
  int nerr = OgSysErr();
  OgSysErrMes(nerr,DOgErrorSize-100,sys_erreur);
  sprintf(erreur,"OgSemaphoreInit: CreateSemaphore error %d: %s",nerr,sys_erreur);
  OgErr(sem->herr,erreur); DPcErr;
  }

DONE;
}



PUBLIC(int) OgSemaphoreWait(sem)
ogsem_t *sem;
{
DWORD dwWaitResult;

IFn(sem) DONE;

dwWaitResult = WaitForSingleObject( 
    sem->hSemaphore,   // handle to semaphore
    INFINITE);         // infinite time-out interval

switch (dwWaitResult) { 
  // The semaphore object was signaled.
  case WAIT_OBJECT_0: break; 
  // Semaphore was nonsignaled, so a time-out occurred.
  case WAIT_TIMEOUT: break; 
  }

DONE;
}



PUBLIC(int) OgSemaphorePost(sem)
ogsem_t *sem;
{
char sys_erreur[DOgErrorSize];
char erreur[DOgErrorSize];

IFn(sem) DONE;

IFn(ReleaseSemaphore( 
    sem->hSemaphore,  // handle to semaphore
    1,           // increase count by one
    NULL)) {     // not interested in previous count
  int nerr = OgSysErr();
  OgSysErrMes(nerr,DOgErrorSize-100,sys_erreur);
  sprintf(erreur,"OgSemaphorePost: ReleaseSemaphore error %d: %s",nerr,sys_erreur);
  OgErr(sem->herr,erreur); DPcErr;
  }

DONE;
}



PUBLIC(int) OgSemaphoreFlush(sem)
ogsem_t *sem;
{
IFn(sem) DONE;
CloseHandle(sem->hSemaphore);
DONE;
}

#endif
#endif




