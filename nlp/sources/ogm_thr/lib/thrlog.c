/*
 *  Mutex protected logging
 *  Copyright (c) 2004 Pertimm bu Patrick Constant
 *  Dev : November 2004
 *  Version 1.0
*/
#include <logthr.h>
#include <stdarg.h>





PUBLIC(int) OgThrMessageLog(ogmutex_t *mutex,int mb,char *title,char * file_ptr,const char * format, ... ) 
{
va_list arg_ptr;
char text[DOgMlogMaxMessageSize+1024];

va_start(arg_ptr,format);
OgEnterCriticalSection(mutex);
vsprintf(text,format,arg_ptr);
OgMessageLog(mb,title,file_ptr,"%s",text);
OgLeaveCriticalSection(mutex);
va_end(arg_ptr);

DONE;
}                





PUBLIC(int) OgThrErrLog(mutex,ptr,where,is_stdout,size_buffer,buffer,flags)
ogmutex_t *mutex; void *ptr; char *where; int is_stdout; 
int size_buffer; char *buffer; int flags;
{
OgEnterCriticalSection(mutex);
OgErrLog(ptr,where,is_stdout,size_buffer,buffer,flags);
OgLeaveCriticalSection(mutex);
DONE;
}



