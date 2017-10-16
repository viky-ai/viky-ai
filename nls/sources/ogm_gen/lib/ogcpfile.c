/*
 *	This is a function that copy files. The function has to
 *  keep the date of the copied file, otherwise, when the filter
 *  copies the file prior to analysing it, the date is wrong
 *  and then, if a rename occurs, date do not match which
 *  implies D+Index instead of only renaming. 
 *  We better use native copy functions because they are
 *  supposed to be more efficient be one nver now...
 *	Copyright (c) 1998-2000	Ogmios by Patrick Constant
 *	Dev : August 1998, April, May 2000
 *	Version 1.2
*/
#include <loggen.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>


#define DOgSizeBuffer	0x8000  /** 32k **/ 


#if (DPcSystem == DPcSystemUnix)
#include <utime.h>


PUBLIC(int) OgCopyFile(char *file_from, char *file_to, int overwrite)
{
int numread;
char erreur[DPcSzErr];
char buffer[DOgSizeBuffer+10];
struct utimbuf modif_time;
FILE *fd_from, *fd_to;
struct stat filestat;

IFn(fd_from=fopen(file_from,"rb")) {
  DPcSprintf(erreur,"OgCopyFile: impossible to open '%s'",file_from);
  PcErr(-1,erreur); DPcErr;
  }

if (!overwrite) {
  if (stat(file_to,&filestat)==0) return(0);
  }
IFn(fd_to=fopen(file_to,"wb")) {
  DPcSprintf(erreur,"OgCopyFile: impossible to open '%s'",file_to);
  PcErr(-1,erreur); DPcErr;
  }

do {
  numread=fread(buffer,1,DOgSizeBuffer,fd_from);
  if (ferror(fd_from)) {
    DPcSprintf(erreur,"Impossible to read '%s'",file_from);
    PcErr(-1,erreur); DPcErr;
    }
  if (numread) {
    fwrite(buffer,1,numread,fd_to);
    if (ferror(fd_to)) {
      DPcSprintf(erreur,"Impossible to write '%s'",file_to);
      PcErr(-1,erreur); DPcErr;
      }
    }
  }
while (numread==DOgSizeBuffer);

if (fstat(fileno(fd_from),&filestat)!=0) {
  DPcSprintf(erreur,"Impossible to fstat '%s'",file_from);
  PcErr(-1,erreur); DPcErr;
  }

fclose(fd_from);
fclose(fd_to);

modif_time.actime=filestat.st_atime;
modif_time.modtime=filestat.st_mtime;

IF(utime(file_to, &modif_time)) {
  DPcSprintf(erreur,"Impossible to change time for '%s'",file_to);
  PcErr(-1,erreur); DPcErr;
  }

return(1);
}

#else
#if (DPcSystem == DPcSystemWin32)


PUBLIC(int) OgCopyFile(file_from,file_to,overwrite)
char *file_from, *file_to; int overwrite;
{
int retour;
char erreur[DPcSzErr];
char sys_erreur[DPcSzSysErr];
int ifile_tow; WCHAR file_tow[DPcPathSize*2];
int ifile_fromw; WCHAR file_fromw[DPcPathSize*2];
IFE(OgUTF8ToUnis(strlen(file_from), file_from, DPcPathSize*2, &ifile_fromw, file_fromw,0,0));
IFE(OgUTF8ToUnis(strlen(file_to), file_to, DPcPathSize*2, &ifile_tow, file_tow,0,0));
retour = CopyFileW(file_fromw,file_tow,!overwrite);
if (!retour) {
  OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
  DPcSprintf(erreur, "OgCopyFile: error copying '%s' into '%s': %s"
            , file_from, file_to, sys_erreur);
  PcErr(-1,erreur); DPcErr;
  }
DONE;
}


#endif
#endif
