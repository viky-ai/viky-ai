/*
 *  Exception Wrapper.
 *  Copyright (c) 2004 Pertimm bu Patrick Constant
 *  Dev : June 2004
 *  Version 1.0
*/
#define DPcInDll
#include <logxpt.h>



#if (DPcSystem == DPcSystemUnix)


/*
 * This function simply call the function 'func' but does not 
 * handle yet any exception.
*/

PUBLIC(int) OgExceptionSafe(func,exception,ptr)
int (*func)(pr(void *));
int (*exception)(pr_(void *) pr(int));
void *ptr; 
{
IFE((*func)(ptr));
DONE;
}


#else
#if (DPcSystem == DPcSystemWin32)


PUBLIC(int) OgExceptionSafe(func,exception,ptr)
int (*func)(pr(void *));
int (*exception)(pr_(void *) pr(int));
void *ptr; 
{
__try {
  /* try block */ 
  IFE((*func)(ptr));
  }
__except (EXCEPTION_EXECUTE_HANDLER) { 
  /* exception handler block */ 
  IFE((*exception)(ptr,GetExceptionCode()));
  } 
DONE;
}

#endif
#endif

