/*
 *	Header for time functions.
 *	Copyright (c) 1997	Ogmios by Patrick Constant
 *	Dev : July 1997
 *	From systal.dll
 *	Version 1.0
*/


#ifndef _LPCTIMEALIVE_

#include <lpcosys.h>


/*
 *      Structure giving time elapsed in 1/100th second.
 *	This structure is used with the function OgCpuTime.
*/

struct scpu_time {
  long user,sys;
  };


DEFPUBLIC(int) OgCpuTime(pr_(char *) pr_(char *) pr(struct scpu_time *));


#define _LPCTIMEALIVE_
#endif


