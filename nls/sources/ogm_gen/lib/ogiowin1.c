/*
 *  CreateFile/SetFilePointer/WriteFile/ReadFile/CloseHandle, 
 *  OgFsync and OgFtell are missing, so there are empty encoded.
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
HANDLE handle;
DWORD dwDesiredAccess = 0;
DWORD dwShareMode = FILE_SHARE_READ + FILE_SHARE_WRITE; 
DWORD dwCreationDistribution = OPEN_EXISTING;
DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
int i,imode = strlen(mode);
char erreur[DPcSzErr];

switch(mode[0]) {
  case 'r': 
    dwDesiredAccess |= GENERIC_READ;
    if (imode > 1 && mode[1]=='+') {
      dwDesiredAccess |= GENERIC_WRITE;
      dwShareMode = FILE_SHARE_READ;
      }
    break;
  case 'w': 
    dwDesiredAccess |= GENERIC_WRITE;
    dwShareMode = FILE_SHARE_READ;
    if (imode > 1 && mode[1]=='+') { 
      dwDesiredAccess |= GENERIC_READ;
      dwCreationDistribution = CREATE_ALWAYS;
      }
    break;
  default:
    sprintf(erreur, "OgFopen: bad mode '%s' on '%s'",mode,filename);	
    PcErr(-1,erreur); return(0);
  }

for (i=0; i<imode; i++) {
  if (mode[i]=='=') {
    //OgMessageLog(DOgMlogInLog,"ogm_ocea",0,"OgFopen on '%s': no cache (mode is '%s')",filename,mode);
    dwFlagsAndAttributes |= FILE_FLAG_NO_BUFFERING;
    }
  else if (mode[i]=='s') {
    //OgMessageLog(DOgMlogInLog,"ogm_ocea",0,"OgFopen on '%s': sequential scan (mode is '%s')",filename,mode);
    dwFlagsAndAttributes |= FILE_FLAG_SEQUENTIAL_SCAN;
    }
  else if (mode[i]=='t') {
    //OgMessageLog(DOgMlogInLog,"ogm_fltr",0,"OgFopen on '%s': write through (mode is '%s')",filename,mode);
    dwFlagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
    }
  }

handle=CreateFile(filename, dwDesiredAccess, dwShareMode, NULL
                  , dwCreationDistribution, dwFlagsAndAttributes, NULL);
if (handle==INVALID_HANDLE_VALUE) return(0);

#ifdef DEVERMINE
OgMessageLog(DOgMlogInLog,"ogm_ocea",0
  ,"OgFopen: HANDLE(%d)=%x",sizeof(HANDLE),handle);
#endif

retour=(void *)handle;
return(retour);
}




PUBLIC(int) OgFstat(stream,filestat)
void *stream; struct og_stat *filestat;
{
int retour, nerr;
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
HANDLE handle = (HANDLE)stream;
FILETIME CreationTime, LastAccessTime, LastWriteTime;

memset(filestat,0,sizeof(struct og_stat));
filestat->size_low = GetFileSize(handle,&filestat->size_high); 
if (filestat->size_low == 0xffffffff && 
    (nerr = GetLastError()) != NO_ERROR ) { 
  /** In case of an error **/
  filestat->size_high = 0;
  filestat->size_low = 0;
  OgSysErrMes(nerr,DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFstat: %s",sys_erreur);	
  PcErr(-1,erreur); DPcErr;
  }

retour = GetFileTime(handle, &CreationTime, &LastAccessTime, &LastWriteTime);
if (retour) {
  IFE(OgFileTimeToTime(&LastWriteTime,&filestat->mtime));
  IFE(OgFileTimeToTime(&CreationTime,&filestat->ctime));
  IFE(OgFileTimeToTime(&LastAccessTime,&filestat->atime));
  }
DONE;
}


PUBLIC(int) OgFseek(stream,offset,origin)
void *stream; struct og_bnum *offset;
int origin;
{
int sys_err;
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
HANDLE handle = (HANDLE)stream;
unsigned offset_low,offset_high;
unsigned position_low,position_high;

IFE(OgBnumToHighLow(offset,&offset_high,&offset_low,1));

position_high = offset_high;

#ifdef DEVERMINE
{
char value[128];
OgBigFormatThousand(offset,value);
OgMessageLog(DOgMlogInLog,"ogm_ocea",0
  ,"OgFseek: HANDLE(%d)=%x high=%x low=%x",sizeof(HANDLE),handle,offset_high,offset_low);
OgMessageLog(DOgMlogInLog,"ogm_ocea",0,"OgFseek: offset = %s",value);
}
#endif

position_low=SetFilePointer(handle,offset_low,&position_high,FILE_BEGIN);
if (position_low==0xffffffff) {
  sys_err=OgSysErr();
  if (sys_err != NO_ERROR) {
    CloseHandle(handle);
    OgSysErrMes(sys_err,DPcSzErr,sys_erreur);
    sprintf(erreur, "OgFseek: %s",sys_erreur);	
    PcErr(-1,erreur); DPcErr;
    }
  }
DONE;
}




/*
 * There is no GetFilePointer, but as documented by Microsoft 
 * (http://msdn.microsoft.com/library/default.asp?url=/library/en-us/fileio/base/positioning_a_file_pointer.asp)
 * we can use SetFilePointer, asking for a zero move from current position.
*/

PUBLIC(int) OgFtell(stream,offset)
void *stream; struct og_bnum *offset;
{
int sys_err;
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
unsigned position_low,position_high=0;
HANDLE handle = (HANDLE)stream;

position_low=SetFilePointer(handle,0,&position_high,FILE_CURRENT);
if (position_low==0xffffffff) {
  sys_err=OgSysErr();
  if (sys_err != NO_ERROR) {
    CloseHandle(handle);
    OgSysErrMes(sys_err,DPcSzErr,sys_erreur);
    sprintf(erreur, "OgFtell: %s",sys_erreur);	
    PcErr(-1,erreur); DPcErr;
    }
  }
IFE(OgHighLowToBnum(position_high,position_low,offset));
DONE;
}






PUBLIC(int) OgFwrite(stream,buffer,size)
void *stream,*buffer; unsigned size;
{
unsigned nb_written;
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
HANDLE handle = (HANDLE)stream;

#ifdef DEVERMINE
OgMessageLog(DOgMlogInLog,"ogm_ocea",0
  ,"OgFwrite: HANDLE(%d)=%x size=%d",sizeof(HANDLE),handle,size);
#endif

if (WriteFile(handle,buffer,size,&nb_written,NULL) == FALSE) {
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFwrite: %s",sys_erreur);	
  PcErr(-1,erreur); CloseHandle(handle); 
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
HANDLE handle = (HANDLE)stream;

if (ReadFile(handle,buffer,size,&nb_read,NULL) == FALSE) {
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFread: %s",sys_erreur);	
  PcErr(-1,erreur); CloseHandle(handle);
  return(0);
  }
return(nb_read);
}




/*
 *  Doing a FlushFileBuffers is a big mistake, because this 
 *  function is called on every words, and takes 100 times 
 *  more time. So we decide not to do anything in this function.
*/

PUBLIC(int) OgFflush(stream)
void *stream;
{
DONE;
}



/*
 *  We have a problem for syncing (committing) the file to the disk
 *  because the handle does not seem to be the same as the one
 *  given by the _open function. And _commit works with _open.
 *  And _open does not handle big files ( > 2Go).
*/

PUBLIC(int) OgFsync(stream)
void *stream;
{
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
HANDLE handle = (HANDLE)stream;

IFn(FlushFileBuffers(handle)) {
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFflush: %s",sys_erreur);	
  PcErr(-1,erreur); CloseHandle(handle);
  return(0);
  }
return(0);
}




PUBLIC(int) OgFclose(stream)
void *stream;
{
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
HANDLE handle = (HANDLE)stream;

#ifdef DEVERMINE
OgMessageLog(DOgMlogInLog,"ogm_ocea",0
  ,"OgFclose: HANDLE(%d)=%x",sizeof(HANDLE),handle);
#endif

if (CloseHandle(handle) == FALSE) {
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFclose: %s",sys_erreur);	
  PcErr(-1,erreur); DPcErr;
  }
DONE;
}



