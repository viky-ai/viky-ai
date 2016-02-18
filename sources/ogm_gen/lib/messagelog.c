/*
 *  Encapsulation of OgOgMessageLog function
 *  Copyright (c) 2000  Ogmios
 *  Dev : March 2000
 *  Version 1.0
*/
#include <loggen.h>


#if (DPcSystem == DPcSystemUnix)
#include <stdio.h>
#include <string.h>


PUBLIC(void) OgOgMessageLog(int mb, char *text, char *textc)
{
sprintf(textc, "%s\nSystem error (%d) %s",text,mb,strerror(mb));
}


PUBLIC(void) OgGetSystemError(int *last_error_number, char *last_system_error)
{
if (last_error_number) *last_error_number=errno;
if (last_system_error) sprintf(last_system_error,"%.*s",DOgErrorSize-1,strerror(errno));
}


#else
#if (DPcSystem == DPcSystemWin32)

#include <windows.h>
#include <direct.h>


PUBLIC(void) OgOgMessageLog(mb, text, textc)
int mb;
char * text;
char * textc;
{
DWORD nerr;
LPVOID lpMsgBuf;

nerr = GetLastError();
FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, nerr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,0,NULL);
sprintf(textc, "%s\nSystem error (%d) %s",text,mb,lpMsgBuf);
LocalFree(lpMsgBuf);
}



PUBLIC(void) OgGetSystemError(last_error_number,last_system_error)
int *last_error_number; char *last_system_error;
{
DWORD nerr;
LPVOID lpMsgBuf;

nerr = GetLastError();
FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, nerr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,0,NULL);
if (last_error_number) *last_error_number=nerr;
if (last_system_error) sprintf(last_system_error,"%.*s",DOgErrorSize-1,lpMsgBuf);
LocalFree(lpMsgBuf);
}

#endif
#endif

