/*
 *  64 bit open/seek/tell/write/read/close 
 *  Buffering is encoded because Microsoft does not provide such a thing.
 *  Copyright (c) 2002 Pertimm, Inc. by Patrick Constant
 *  Dev : February 2002
 *  Version 1.0
*/
#include <fcntl.h>
#include <io.h>



/*
 * 'sB' is the size of the buffer 'B'. 
 * If sB is zero, there is not buffered io.
 * 'pos' is the position in the file
 * 'posB' is the position of the beginning of buffer 'B' in the file
 * 'wB' says the buffer need to be written to the disk.  
 * 'rB' says the buffer has been read from the disk.  
*/

struct og_fio {
  int handle;
  int sB,iB,wB,rB; char *B;
  __int64 pos, posB;
  };

static int OgIoBufferSize = 0;


static int Fread(struct og_fio *, char *, unsigned);
static int FreadB(struct og_fio *, __int64);
static int Fwrite(struct og_fio *, char *, unsigned);
static int FwriteB(struct og_fio *);



/*
 * Sets the buffer size for OgFopen. This function should
 * be started just before calling OgFopen and then OgFopen
 * resets the value to zero (the default value). This avoids
 * and remanent effect on that function. This function has no
 * effect on other system than Windows.
*/

PUBLIC(int) OgSetIoBufferSize(int size)
{
OgIoBufferSize = size;
DONE;
}



PUBLIC(void *) OgFopen(char *filename, char *mode)
{
void *retour;
int handle,oflag=0,pmode=0;
int i,imode = strlen(mode);
char erreur[DPcSzErr];
struct og_fio *fio;

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

IFn(fio=(struct og_fio *)malloc(sizeof(struct og_fio))) {
  sprintf(erreur, "OgFopen: could not malloc %d bytes",sizeof(struct og_fio));	
  PcErr(-1,erreur); return(0);
  }

memset(fio,0,sizeof(struct og_fio));
fio->sB = OgIoBufferSize;
fio->handle = handle;

//if (fio->sB>=0) { 
if (fio->sB>0) { 
  IFn(fio->B=(char *)malloc(fio->sB+10)) {
    sprintf(erreur, "OgFopen: could not malloc %d bytes",fio->sB);	
    PcErr(-1,erreur); close(handle); return(0);
    }
  /** To avoid remanent effect **/
  OgIoBufferSize = 0;
  }

retour=(void *)fio;
return(retour);
}



PUBLIC(int) OgFstat(void *stream, struct og_stat *filestat)
{
int retour;
struct _stati64 orig_filestat;
struct og_fio *fio = (struct og_fio *)stream;

retour = _fstati64(fio->handle,&orig_filestat);
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






PUBLIC(int) OgFseek(void *stream, struct og_bnum *offset, int origin)
{
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
struct og_fio *fio = (struct og_fio *)stream;
__int64 offset64;

IFE(OgBnumToLong64(offset,&offset64,1));

if (fio->sB && fio->rB && fio->posB <= offset64 && offset64 < fio->posB + fio->iB) {
  fio->pos = offset64;
  }
else {
  fio->pos = _lseeki64(fio->handle, offset64, SEEK_SET);
  if (fio->pos == (__int64)(-1)) {
    close(fio->handle);
    OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
    sprintf(erreur, "OgFseek: %s",sys_erreur);	
    PcErr(-1,erreur); DPcErr;
    }
  }
DONE;
}




PUBLIC(int) OgFtell(void *stream, struct og_bnum *offset)
{
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
struct og_fio *fio = (struct og_fio *)stream;
__int64 offset64;

if (fio->sB) {
  offset64 = fio->pos;
  }
else {
  offset64 = _telli64(fio->handle);
  if (offset64 == (__int64)(-1)) {
    close(fio->handle);
    OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
    sprintf(erreur, "OgFtell: %s",sys_erreur);	
    PcErr(-1,erreur); DPcErr;
    }
  }
IFE(OgLong64ToBnum(offset64,offset));
DONE;
}







PUBLIC(int) OgFwrite(void *stream, char *buffer, unsigned size)
{
unsigned nb_written;
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
struct og_fio *fio = (struct og_fio *)stream;
int length1, length2;

if (fio->sB) {
  /** pos is within the buffer **/
  if (fio->rB && fio->posB <= fio->pos && fio->pos < fio->posB + fio->sB) {
    int start = (int)(fio->pos - fio->posB);
    if ((int)size < fio->sB - start) {
      memcpy(fio->B+start,buffer,size); fio->wB=1;
      if (fio->iB < (int)(start+size)) fio->iB = start+size;
      nb_written = size; fio->pos += nb_written;
      }
    else {
      length1 = fio->sB - start;
      memcpy(fio->B+start,buffer,length1); 
      if (fio->iB < start+length1) fio->iB = start+length1;
      fio->wB=1; IFE(FwriteB(fio));
      length2 = size - length1;
      fio->pos += length1;
      IFE(nb_written=Fwrite(fio,buffer+length1,length2));
      fio->pos += nb_written;
      nb_written += length1;
      }
    }
  else {
    if ((int)size >= fio->sB) {
      IFE(nb_written=Fwrite(fio,buffer,size));
      fio->pos += nb_written;
      }
    else {
      IFE(FreadB(fio,fio->pos));
      memcpy(fio->B,buffer,size); fio->wB=1;
      if (fio->iB < (int)size) fio->iB = size;
      nb_written = size;      
      fio->pos += nb_written;
      }
    }
  }
else {
  IF(nb_written=write(fio->handle,buffer,size)) {
    OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
    sprintf(erreur, "OgFwrite: %s",sys_erreur);	
    PcErr(-1,erreur); close(fio->handle); 
    return(0);
    }
  }
return(nb_written);
}




static int Fwrite(struct og_fio *fio, char *buffer, unsigned size)
{
unsigned nb_written;
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];

if (fio->sB) {
  __int64 offset64;
  offset64 = _lseeki64(fio->handle, fio->pos, SEEK_SET);
  if (offset64 == (__int64)(-1)) {
    OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
    sprintf(erreur, "OgFseek: %s",sys_erreur);	
    PcErr(-1,erreur); close(fio->handle);
    DPcErr;
    }
  }

IF(nb_written=write(fio->handle,buffer,size)) {
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFwrite: %s",sys_erreur);	
  PcErr(-1,erreur); close(fio->handle); 
  DPcErr;
  }
return(nb_written);
}





static int FwriteB(struct og_fio *fio)
{
int nb_written;
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
__int64 offset64;

offset64 = _lseeki64(fio->handle, fio->posB, SEEK_SET);
if (offset64 == (__int64)(-1)) {
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFseek: %s",sys_erreur);	
  PcErr(-1,erreur); close(fio->handle);
  DPcErr;
  }

IF(nb_written=write(fio->handle,fio->B,fio->iB)) {
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFwrite: %s",sys_erreur);	
  PcErr(-1,erreur); close(fio->handle); 
  DPcErr;
  }
if (nb_written != fio->iB) {
  sprintf(erreur, "FwriteB: nb_written (%d) != fio->iB (%d)",nb_written,fio->iB);	
  PcErr(-1,erreur); close(fio->handle); 
  DPcErr;
  }
fio->wB = 0;
fio->rB = 0;
DONE;
}








PUBLIC(int) OgFread(void *stream, char *buffer, unsigned size)
{
unsigned nb_read=0;
struct og_fio *fio = (struct og_fio *)stream;
int length1, length2;

if (fio->sB) {
  /** pos is within the buffer **/
  if (fio->rB && fio->posB <= fio->pos && fio->pos < fio->posB + fio->iB) {
    int start = (int)(fio->pos - fio->posB);
    if ((int)size < fio->iB - start) {
      memcpy(buffer,fio->B+start,size);
      nb_read = size;
      }
    else {
      length1 = fio->iB - start;
      memcpy(buffer,fio->B+start,length1);
      length2 = size - length1;
      if (fio->iB == fio->sB) {
        if (length2 >= fio->sB) {
          IFE(nb_read=Fread(fio,buffer+length1,length2));
          fio->rB = 0; nb_read += length1;
          }
        else {
          IFE(FreadB(fio, fio->pos+length1));
          if (length2 > fio->iB) length2 = fio->iB;
          memcpy(buffer+length1,fio->B,length2);
          nb_read = length1 + length2;
          }
        }
      else {
        nb_read = length1;
        }
      }    
    }
  else {
    if ((int)size >= fio->sB) {
      IFE(nb_read=Fread(fio,buffer,size));
      fio->rB = 0;
      }
    else {
      IFE(FreadB(fio,fio->pos));
      if ((int)size > fio->iB) size = fio->iB;
      memcpy(buffer,fio->B,size);
      nb_read = size;
      }
    }
  }
else {
  IFE(nb_read=Fread(fio,buffer,size));
  }
fio->pos += nb_read;
return(nb_read);
}



#define DOgReadLoopOnLockViolation  100

static int Fread(struct og_fio *fio, char *buffer, unsigned size)
{
int i,nb_read,lock_violation=0;
char sys_erreur[DPcSzErr];
char erreur[DPcSzErr*2];

if (fio->sB) {
  __int64 offset64;
  offset64 = _lseeki64(fio->handle, fio->pos, SEEK_SET);
  if (offset64 == (__int64)(-1)) {
    OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
    sprintf(erreur, "OgFseek: %s",sys_erreur);	
    PcErr(-1,erreur); close(fio->handle);
    DPcErr;
    }
  }

/* Attempt to escape from a lock operation, October 8th 2004
 * Such a case was met on the EIG project with Alogic and a 2Gb repository.
 * Very rare, we wait for 1 second for the lock to be released. */
for (i=0; i<DOgReadLoopOnLockViolation; i++) {
  IF(nb_read=read(fio->handle,buffer,size)) {
    int err_number=OgSysErr();
    if (err_number!=ERROR_LOCK_VIOLATION || i>=DOgReadLoopOnLockViolation-1) {
      OgSysErrMes(err_number,DPcSzErr,sys_erreur);
      sprintf(erreur, "Fread errno=%d: %s",sys_erreur);	
      PcErr(-1,erreur); close(fio->handle);
      return(0);
      }
    OgSleep(10); lock_violation=1;
    }
  else break;
  }

if (lock_violation) {
  OgMessageLog(DOgMlogInLog+DOgMlogDateIn,"ogfread",0
    ,"Fread: waited %d milli-seconds for end of ERROR_LOCK_VIOLATION (nb_read=%d pos=%ld)"
    , (i+1)*10, nb_read, fio->pos);
  }

return(nb_read);
}





static int FreadB(struct og_fio *fio, __int64 pos)
{
int i,nb_read,lock_violation=0;
char sys_erreur[DPcSzErr];
char erreur[DPcSzErr*2];

if (fio->wB) { IFE(FwriteB(fio)); }

if (pos != fio->pos) {
  __int64 offset64;
  offset64 = _lseeki64(fio->handle, pos, SEEK_SET);
  if (offset64 == (__int64)(-1)) {
    OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
    sprintf(erreur, "OgFseek: %s",sys_erreur);	
    PcErr(-1,erreur); close(fio->handle);
    DPcErr;
    }
  }

/* Attempt to escape from a lock operation, October 8th 2004
 * Such a case was met on the EIG project with Alogic and a 2Gb repository.
 * Very rare, we wait for 1 second for the lock to be released. */
for (i=0; i<DOgReadLoopOnLockViolation; i++) {
  IF(nb_read=read(fio->handle,fio->B,fio->sB)) {
    int err_number=OgSysErr();
    if (err_number!=ERROR_LOCK_VIOLATION || i>=DOgReadLoopOnLockViolation-1) {
      OgSysErrMes(err_number,DPcSzErr,sys_erreur);
      sprintf(erreur, "FreadB errno=%d: %s",sys_erreur);	
      PcErr(-1,erreur); close(fio->handle);
      DPcErr;
      }
    OgSleep(10); lock_violation=1;
    }
  else break;
  }

if (lock_violation) {
  OgMessageLog(DOgMlogInLog+DOgMlogDateIn,"ogfread",0
    ,"FreadB: waited %d milli-seconds for end of ERROR_LOCK_VIOLATION (nb_read=%d pos=%ld)"
    , (i+1)*10, nb_read, pos);
  }

fio->iB = fio->sB;
if (fio->iB > nb_read) fio->iB = nb_read; 
fio->wB = 0; fio->rB = 1;
fio->posB = pos;

DONE;
}





PUBLIC(int) OgFflush(void *stream)
{
struct og_fio *fio = (struct og_fio *)stream;
if (fio->sB && fio->wB) {
  IFE(FwriteB(fio));
  }
DONE;
}




PUBLIC(int) OgFsync(void *stream)
{
int retour;
int sys_err;
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
struct og_fio *fio = (struct og_fio *)stream;

retour = _commit(fio->handle);
if (retour==0) DONE;

sys_err=OgSysErr();
OgSysErrMes(sys_err,DPcSzErr,sys_erreur);
sprintf(erreur, "OgFsync: %s",sys_erreur);	
PcErr(-1,erreur); DPcErr;
}




PUBLIC(int) OgFclose(void *stream)
{
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
struct og_fio *fio = (struct og_fio *)stream;

if (fio->sB) {
  if (fio->wB) { IFE(FwriteB(fio)); }
  }

IF(close(fio->handle)) {
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFclose: %s",sys_erreur);	
  PcErr(-1,erreur); DPcErr;
  }
if (fio->sB) {
  DPcFree(fio->B);
  }
DPcFree(fio);

DONE;
}



