/*
 *  Terminating threads. Warning: this code has not been tested
 *  Copyright (c) 2004 Pertimm bu Patrick Constant
 *  Dev : March 2004
 *  Version 1.0
*/
#include <logthr.h>



#if (DPcSystem == DPcSystemUnix)
#include <signal.h>


PUBLIC(int) OgTerminateThread(IT)
ogthread_t *IT;
{
int nerr;
char sys_erreur[DPcSzErr], erreur[DPcSzErr]; 
IFn(pthread_cancel(IT->hThread)) DONE;
nerr = OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
sprintf(erreur,"OgTerminateThread: could not terminate thread: (%d) %s\n",nerr,sys_erreur);
PcErr(-1,erreur); DPcErr; 
}


#else
#if (DPcSystem == DPcSystemWin32)


PUBLIC(int) OgTerminateThread(IT)
ogthread_t *IT;
{
int nerr;
char sys_erreur[DPcSzErr], erreur[DPcSzErr]; 
if (TerminateThread(IT->hThread,127)) DONE;
nerr = OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
sprintf(erreur,"OgTerminateThread: could not terminate thread: (%d) %s\n",nerr,sys_erreur);
PcErr(-1,erreur); DPcErr; 
}


#endif
#endif




