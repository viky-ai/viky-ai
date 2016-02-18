/*
 *  Sending errors
 *  Copyright (c) 2007 Pertimm by Patrick Constant and Guillaume Logerot
 *  Dev : March,April 2007
 *  Version 1.1
*/
#include "ogm_msg.h"



/*
 *  This is a copy of the OgErrLog using OgMsg instead of OgMessageLog
 *  flags is the OgErrLog flags. It is not possible to encapsulate
 *  OgMessageLog as OgMsg can send information other than in
 *  log files.
*/

PUBLIC(int) OgMsgErr(void *handle, char *id, int is_stdout, int size_buffer, char *buffer, int mb, int flags)
{
struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;
char last_error[DOgErrorSize],b[DOgErrorSize*2];
int last_error_number; char *cr, last_system_error[DOgErrorSize];
int n,h,is_error=0; time_t error_time;
char header[DOgErrorSize],date[DOgErrorSize];
int error_number=0;
int ibuffer=0;

if (buffer) buffer[0]=0;

if (flags & DOgErrLogFlagNotInErr) mb|=DOgMsgDestInLog;
else mb|=DOgMsgDestInLog+DOgMsgDestInErr;

if (flags & DOgErrLogFlagCrIsBr) cr="<br>";
else cr="\n";

IFx(ctrl_msg->herr) {
  error_number = OgErrNumber(ctrl_msg->herr); n=1;
  while(OgErrLast(ctrl_msg->herr,last_error,&error_time)) {
    if (flags & DOgErrLogFlagNoHeader) header[0]=0;
    else sprintf(header,"Error %d/%d: ", n, error_number);
    if (flags & DOgErrLogFlagNoDate) date[0]=0;
    else sprintf(date," at %.24s UTC", OgGmtime(&error_time));
    sprintf(b,"%s%s%s", header, last_error, date);
    OgMsg(ctrl_msg,id,mb,"%s", b);
    if (is_stdout) printf("%s\n", b);
    if (buffer) {
      snprintf(buffer+ibuffer,size_buffer-ibuffer,"%s%s", n>1?cr:"",b);
      ibuffer=strlen(buffer);
      }
    is_error=1; n++;
    }
  }

while(PcErrDiag(&h,last_error)) {
  if (flags & DOgErrLogFlagNoHeader) header[0]=0;
  else sprintf(header,"Old style error: ");
  sprintf(b,"%s%s", header, last_error);
  OgMsg(ctrl_msg,id,mb,"%s", b);
  if (is_stdout) printf("%s\n", b);
  if (buffer) {
    snprintf(buffer+ibuffer,size_buffer-ibuffer,"%s%s",cr,b);
    ibuffer=strlen(buffer);
    }
  is_error=1;
  }

if (!(flags & DOgErrLogFlagNoSystemError)) {
  if (is_error) {
    time(&error_time);
    OgGetSystemError(&last_error_number,last_system_error);
    if (ctrl_msg->loginfo->where) {
      OgMsg(ctrl_msg,id,mb
        ,"System error at %.24s UTC (%d): %.*s", OgGmtime(&error_time), last_error_number, DOgErrorSize-100, last_system_error);
      }
    if (is_stdout) {
      printf("System error at %.24s UTC (%d): %.*s\n", OgGmtime(&error_time), last_error_number, DOgErrorSize-100, last_system_error);
      }
    if (buffer) {
      snprintf(buffer+ibuffer,size_buffer-ibuffer,"%sSystem error at %.24s UTC (%d): %.*s", cr, OgGmtime(&error_time),
          last_error_number, DOgErrorSize-100, last_system_error);
      ibuffer=strlen(buffer);
      }
    }
  }

return(is_error);
}



