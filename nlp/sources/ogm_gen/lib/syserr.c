/*
 *	Handling of system errors
 *	Encapsulation of OgSysErrMes function
 *	Copyright (c) 1999	Ogmios by Patrick Constant
 *	Dev : October 1999
 *	Version 1.0
*/
#include <loggen.h>
#include <string.h>


#if (DPcSystem == DPcSystemUnix)

#include <errno.h>
extern int errno;

PUBLIC(int) OgSysErr(void)
{
  return errno;
}

PUBLIC(int) OgSysErrMes(int nerr, int iB, char *B)
{
int length; char sys_erreur[DPcSzErr];

sprintf(sys_erreur, "System error (%d) %s",nerr,strerror(nerr));

length = strlen(sys_erreur);
if (sys_erreur[length-1] == '\n') length--;
if (sys_erreur[length-1] == '\r') length--;
sprintf(B, "%.*s", (length>iB)?iB:length, sys_erreur);
return(nerr);
}

#else
#if (DPcSystem == DPcSystemWin32)

#include <windows.h>

PUBLIC(int) OgSysErr()
{
  return(GetLastError());
}

PUBLIC(int) OgSysErrMes(nerr,iB,B)
int nerr,iB; char *B;
{
int length; char sys_erreur[DPcSzErr];
FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, nerr
             , MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
             , sys_erreur,DPcSzErr,NULL);
length = strlen(sys_erreur);
if (sys_erreur[length-1] == '\n') length--;
if (sys_erreur[length-1] == '\r') length--;
sprintf(B, "%.*s", (length>iB)?iB:length, sys_erreur);
return(nerr);
}

#endif
#endif
