/*
 *  General error handling, thread safe.
 *  Copyright (c) 2004 Pertimm by Patrick Constant
 *  Dev : November,December 2004
 *  Version 1.0
*/
#include <stdarg.h>
#include <loggen.h>


#define  DOgErreurNumber     30


struct og_erreur {
  time_t time;
  char ch[DOgErrorSize];
  };

struct og_ctrl_err {
  THREAD_MUTEX mutex[1];
  unsigned ErreurUsed;
  struct og_erreur Erreur[DOgErreurNumber+1];
  };



/*
 *  Returns a pointer to the allocated control structure.
 *  Returns 0 on error. The only possible error is on the allocation
 *  This is why we use an array of a definite length. We also
 *  do not use logs because it can create errors too.
*/

PUBLIC(void *) OgErrInit(void)
{
struct og_ctrl_err *ctrl_err;

IFn(ctrl_err = (struct og_ctrl_err *)malloc(sizeof(struct og_ctrl_err))) return(0);
memset(ctrl_err,0,sizeof(struct og_ctrl_err));
OgInitMutex(ctrl_err->mutex);

return((void *)ctrl_err);
}





PUBLIC(int) OgErrFlush(void *ptr)
{
struct og_ctrl_err *ctrl_err = (struct og_ctrl_err *)ptr;

IFn(ptr) DONE;

OgDelMutex(ctrl_err->mutex);
DPcFree(ctrl_err);
DONE;
}




/*
 * lastest error is always the last one.
 * We keep only the latest DOgErreurNumber errors
*/

PUBLIC(int) OgErr(void *ptr, char *new_erreur)
{
struct og_ctrl_err *ctrl_err = (struct og_ctrl_err *)ptr;
struct og_erreur *erreur;
int inew_erreur;

IFn(ptr) {
  PcErr(-1,new_erreur);
  DONE;
  }
OgLockMutex(ctrl_err->mutex);

if (ctrl_err->ErreurUsed >= DOgErreurNumber) {
  memmove(ctrl_err->Erreur,ctrl_err->Erreur+1,(DOgErreurNumber-1)*sizeof(struct og_erreur));
  ctrl_err->ErreurUsed = DOgErreurNumber-1;
  }

inew_erreur = strlen(new_erreur);
if (inew_erreur >= DOgErrorSize-1) inew_erreur = DOgErrorSize-1;
erreur = ctrl_err->Erreur + ctrl_err->ErreurUsed;
memcpy(erreur->ch,new_erreur,inew_erreur);
erreur->ch[inew_erreur]=0;
time(&erreur->time);

ctrl_err->ErreurUsed++;
OgUnlockMutex(ctrl_err->mutex);
DONE;
}






PUBLIC(int) OgErrLast(void *ptr, char *last_erreur, time_t *ptime)
{
struct og_ctrl_err *ctrl_err = (struct og_ctrl_err *)ptr;
struct og_erreur *erreur;

IFn(ptr) {
  PcErrLast(-1,last_erreur);
  DONE;
  }
OgLockMutex(ctrl_err->mutex);

if (ctrl_err->ErreurUsed <= 0)
{
  OgUnlockMutex(ctrl_err->mutex);
  return(0);
}

erreur = ctrl_err->Erreur + ctrl_err->ErreurUsed-1;
IFx(last_erreur) strcpy(last_erreur,erreur->ch);
IFx(ptime) *ptime=erreur->time;

ctrl_err->ErreurUsed--;
OgUnlockMutex(ctrl_err->mutex);

return(1);
}





PUBLIC(int) OgErrReset(void *ptr)
{
struct og_ctrl_err *ctrl_err = (struct og_ctrl_err *)ptr;
IFn(ptr) DONE;
OgLockMutex(ctrl_err->mutex);
ctrl_err->ErreurUsed=0;
OgUnlockMutex(ctrl_err->mutex);
DONE;
}





PUBLIC(int) OgErrNumber(void *ptr)
{
struct og_ctrl_err *ctrl_err = (struct og_ctrl_err *)ptr;
IFn(ptr) return(0);
return(ctrl_err->ErreurUsed);
}





/*
 * There is a function called OgThrErrLog in the thread library
 * that does the same thing but uses a mutex for thread safe logs.
 * This function could have been encoded in any library since it
 * does not use the og_ctrl_err structure.
*/

PUBLIC(int) OgErrLog(void *ptr, char *where, int is_stdout, int size_buffer, char *buffer, int flags)
{
char last_error[DOgErrorSize],local_buffer[DOgErrorSize*2],b[DOgErrorSize*2];
int last_error_number; char *cr, last_system_error[DOgErrorSize];
int n,h,is_error=0,ilocal_buffer,ibuffer; time_t error_time;
char header[DOgErrorSize],date[DOgErrorSize];
int buffer_overflow=0,error_number,mb;

if (buffer) buffer[0]=0;

if (flags & DOgErrLogFlagNotInErr) mb=DOgMlogInLog;
else mb=DOgMlogInLog+DOgMlogInErr;

if (flags & DOgErrLogFlagCrIsBr) cr="<br>";
else cr="\n";

IFx(ptr) {
  error_number = OgErrNumber(ptr); n=1;
  while(OgErrLast(ptr,last_error,&error_time)) {
    if (flags & DOgErrLogFlagNoHeader) header[0]=0;
    else sprintf(header,"Error %d/%d: ", n, error_number);
    if (flags & DOgErrLogFlagNoDate) date[0]=0;
    else sprintf(date," at %.24s", OgGmtime(&error_time));
    sprintf(b,"%s%s%s", header, last_error, date);

    if (where) OgMessageLog(mb,where,0,"%s", b);
    if (is_stdout) printf("%s\n", b);
    if (buffer && !buffer_overflow) {
      sprintf(local_buffer,"%s%s", b,cr);
      ilocal_buffer=strlen(local_buffer); ibuffer=strlen(buffer);
      if (ibuffer+ilocal_buffer >= size_buffer-1) buffer_overflow=1;
      if (!buffer_overflow) sprintf(buffer+ibuffer,"%s",local_buffer);
      }
    is_error=1; n++;
    }
  }

while(PcErrDiag(&h,last_error)) {
  if (flags & DOgErrLogFlagNoHeader) header[0]=0;
  else sprintf(header,"Old style error: ");
  sprintf(b,"%s%s", header, last_error);
  if (where) OgMessageLog(mb,where,0,"%s", b);
  if (is_stdout) printf("%s\n", b);
  if (buffer && !buffer_overflow) {
    sprintf(local_buffer,"%s%s", b,cr);
    ilocal_buffer=strlen(local_buffer); ibuffer=strlen(buffer);
    if (ibuffer+ilocal_buffer >= size_buffer-1) buffer_overflow=1;
    if (!buffer_overflow) sprintf(buffer+ibuffer,"%s",local_buffer);
    }
  is_error=1;
  }

if (!(flags & DOgErrLogFlagNoSystemError)) {
  if (is_error) {
    time(&error_time);
    OgGetSystemError(&last_error_number,last_system_error);
    if (where) {
      OgMessageLog(mb,where,0
        ,"System error at %.24s (%d): %.*s", OgGmtime(&error_time), last_error_number, DOgErrorSize-100, last_system_error);
      }
    if (is_stdout) {
      printf("System error at %.24s (%d): %.*s\n", OgGmtime(&error_time), last_error_number, DOgErrorSize-100, last_system_error);
      }
    if (buffer && !buffer_overflow) {
      sprintf(local_buffer
        ,"System error at %.24s (%d): %.*s%s", OgGmtime(&error_time)
          , last_error_number, DOgErrorSize-100, last_system_error, cr);
      ilocal_buffer=strlen(local_buffer); ibuffer=strlen(buffer);
      if (ibuffer+ilocal_buffer >= size_buffer-1) buffer_overflow=1;
      if (!buffer_overflow) sprintf(buffer+ibuffer,"%s",local_buffer);
      }
    }
  }

return(is_error);
}


PUBLIC(int) _OgErrMsg(void *handle, char *srcFile, int line, char *format, ...)
{
va_list arg_ptr;
static char msg1[DOgErrorSize];
static char msg2[DOgErrorSize];

va_start(arg_ptr,format);
vsprintf(msg1,format,arg_ptr);
va_end(arg_ptr);
snprintf(msg2,DOgErrorSize-1,"%s:%d:%s",srcFile,line,msg1); msg2[DOgErrorSize-1]='\0';
OgErr(handle,msg2);
DONE;
}





PUBLIC(size_t) OgErrGetAllocatedMemory(void *handle)
{
return(sizeof(struct og_ctrl_err));
}



PUBLIC(int) OgErrGetIeme(void *ptr, char *last_erreur, unsigned ieme_error, time_t *ptime)
{
  struct og_ctrl_err *ctrl_err = (struct og_ctrl_err *) ptr;

  OgLockMutex(ctrl_err->mutex);

  if (ieme_error > ctrl_err->ErreurUsed)
  {
    OgUnlockMutex(ctrl_err->mutex);
    return 0;
  }

  struct og_erreur *erreur = ctrl_err->Erreur + ieme_error;
  IFx(last_erreur) snprintf(last_erreur, DOgErrorSize, "%s", erreur->ch);
  IFx(ptime) *ptime = erreur->time;

  OgUnlockMutex(ctrl_err->mutex);

  return 1;
}
