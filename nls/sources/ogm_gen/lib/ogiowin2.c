/*
 *  64 bit open/seek/tell/write/read/close 
 *  It can be slow in some case because there is no buffering.
 *  Copyright (c) 2002 Pertimm, Inc. by Patrick Constant
 *  Dev : February 2002
 *  Version 1.0
*/
#include <fcntl.h>
#include <io.h>



/** useless except for ogoiwin3.c **/
PUBLIC(int) OgSetIoBufferSize(size)
int size;
{
DONE;
}


/*
 *  The Unix Version is working on Windows 95/98/NT, but the native Win32
 *  functions are more powerful and can go beyond a file size of 4 giga-bytes. 
 *  For the moment, we limit the 'mode' interpretation to "r" "w" "r+" "w+"
 *  and a binary opening of the file.
*/

PUBLIC(void *) OgFopen(filename,mode)
char *filename, *mode;
{
void *retour;
int handle,oflag=0,pmode=0;
int i,imode = strlen(mode);
char erreur[DPcSzErr];

switch(mode[0]) {
  case 'r': 
    oflag = O_RDONLY;
    if (imode > 1 && mode[1]=='+') oflag = O_RDWR;
    break;
  case 'w':
    oflag = O_TRUNC|O_CREAT;
    if (imode > 1 && mode[1]=='+') oflag |= O_RDWR;
    else oflag |= O_WRONLY;
    break;
  case 'a':
    oflag = O_APPEND;
    if (imode > 1 && mode[1]=='+') oflag = O_RDWR;
    break;
  default:
    sprintf(erreur, "OgFopen: bad mode '%s' on '%s'",mode,filename);	
    PcErr(-1,erreur); return(0);
  }

for (i=0; i<imode; i++) {
  if (mode[i]=='b') oflag |= O_BINARY;
  }

IF(handle=open(filename, oflag, _S_IREAD|_S_IWRITE)) return(0);

retour=(void *)handle;
return(retour);
}





PUBLIC(int) OgFstat(stream,filestat)
void *stream; struct og_stat *filestat;
{
int retour;
int handle = (int)stream;
struct _stati64 orig_filestat;
retour = _fstati64(handle,&orig_filestat);
if (retour==0) {
  filestat->atime = orig_filestat.st_atime;
  filestat->mtime = orig_filestat.st_mtime;
  filestat->ctime = orig_filestat.st_ctime;
  filestat->size_high = (int)(orig_filestat.st_size>>32);
  filestat->size_low = (int)(orig_filestat.st_size & 0xffffffff);
  if (OgIsDir(orig_filestat.st_mode)) filestat->is_dir  = 1;
  else filestat->is_dir  = 0;
  }
return(retour);
}






PUBLIC(int) OgFseek(stream,offset,origin)
void *stream; struct og_bnum *offset;
int origin;
{
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
int handle = (int)stream;
__int64 offset64,position64;

IFE(OgBnumToLong64(offset,&offset64,1));

position64 = _lseeki64(handle, offset64, SEEK_SET);
if (position64 == (__int64)(-1)) {
  close(handle);
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFseek: %s",sys_erreur);	
  PcErr(-1,erreur); DPcErr;
  }
DONE;
}




PUBLIC(int) OgFtell(stream,offset)
void *stream; struct og_bnum *offset;
{
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
int handle = (int)stream;
__int64 offset64;

offset64 = _telli64(handle);
if (offset64 == (__int64)(-1)) {
  close(handle);
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFtell: %s",sys_erreur);	
  PcErr(-1,erreur); DPcErr;
  }
IFE(OgLong64ToBnum(offset64,offset));
DONE;
}





PUBLIC(int) OgFwrite(stream,buffer,size)
void *stream,*buffer; unsigned size;
{
unsigned nb_written;
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
int handle = (int)stream;

IF(nb_written=write(handle,buffer,size)) {
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFwrite: %s",sys_erreur);	
  PcErr(-1,erreur); close(handle); 
  return(0);
  }
return(nb_written);
}






PUBLIC(int) OgFread(stream,buffer,size)
void *stream,*buffer; unsigned size;
{
unsigned nb_read;
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
int handle = (int)stream;

IF(nb_read=read(handle,buffer,size)) {
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFread: %s",sys_erreur);	
  PcErr(-1,erreur); close(handle);
  return(0);
  }
return(nb_read);
}




PUBLIC(int) OgFflush(stream)
void *stream;
{
/** not applicable **/
DONE;
}





PUBLIC(int) OgFclose(stream)
void *stream;
{
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
int handle = (int)stream;

IF(close(handle)) {
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFclose: %s",sys_erreur);	
  PcErr(-1,erreur); DPcErr;
  }
DONE;
}






PUBLIC(int) OgFsync(stream)
void *stream;
{
int retour;
int sys_err;
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
int handle = (int)stream;

retour = _commit(handle);
if (retour==0) DONE;

sys_err=OgSysErr();
OgSysErrMes(sys_err,DPcSzErr,sys_erreur);
sprintf(erreur, "OgFsync: %s",sys_erreur);	
PcErr(-1,erreur); DPcErr;
}


