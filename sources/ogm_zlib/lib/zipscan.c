/*
 *  The heart function for handling Zip files.
 *  This is fully based upon the zlib122/contrib/miniunz.c program.
 *  Copyright (c) 2005 Pertimm by Patrick Constant
 *  Dev : April 2005
 *  Version 1.0
*/
#include "ogm_zlib.h"
#include "unzip.h"

#ifdef WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif


STATICF(int) OgZipScan1(pr_(struct og_ctrl_zlib *) pr_(unzFile) pr(char *));
STATICF(int) OgZipScanListingAdd(pr_(struct og_ctrl_zlib *) pr_(unzFile)
                                 pr_(char *) pr_(char *) pr(unz_file_info *));




PUBLIC(int) OgZlibIsZip(og_zip_lib handle, char *filename)
{
struct og_ctrl_zlib *ctrl_zlib = (struct og_ctrl_zlib *)handle;
char erreur[DOgErrorSize];
unzFile uf; int retour;

#ifdef USEWIN32IOAPI
zlib_filefunc_def ffunc;
fill_win32_filefunc(&ffunc);
uf = unzOpen2(filename,&ffunc);
#else
uf = unzOpen(filename);
#endif
IFn(uf) return(0);

retour = unzClose(uf);
if (retour!=UNZ_OK) {
  sprintf(erreur,"OgZlibIsZip: unzClose: error %d for '%s'",retour,ctrl_zlib->zlib_name);
  OgErr(ctrl_zlib->herr,erreur); DPcErr;
  }

return(1);
}





PUBLIC(int) OgZipScan(og_zip_lib handle, struct og_zlib_scan_param *zlib_scan_param)
{
struct og_ctrl_zlib *ctrl_zlib = (struct og_ctrl_zlib *)handle;
char filename_inzip[DPcPathSize];
unzFile uf; unz_global_info gi;
char erreur[DOgErrorSize];
int i,retour,is; char *s;

#ifdef USEWIN32IOAPI
zlib_filefunc_def ffunc;
#endif

ctrl_zlib->scan_param = &ctrl_zlib->cscan_param;
memcpy(ctrl_zlib->scan_param,zlib_scan_param,sizeof(struct og_zlib_scan_param));

strcpy(ctrl_zlib->zlib_name,ctrl_zlib->scan_param->zlib_name);
ctrl_zlib->zlib_part[0]=0;

s = ctrl_zlib->zlib_name; is=strlen(s);
for (i=0; i+5<is; i++) {
  if (memcmp(s+i,"\1zip\1",5)) continue;
  s[i]=0; strcpy(ctrl_zlib->zlib_part,s+i+5);
  break;
  }

ctrl_zlib->BaUsed = 0;
ctrl_zlib->ZfileUsed = 0;

#ifdef USEWIN32IOAPI
fill_win32_filefunc(&ffunc);
uf = unzOpen2(ctrl_zlib->zlib_name,&ffunc);
#else
uf = unzOpen(ctrl_zlib->zlib_name);
#endif

IFn(uf) {
  sprintf(erreur,"OgZipScan: impossible to unzOpen '%s'",ctrl_zlib->zlib_name);
  OgErr(ctrl_zlib->herr,erreur); DPcErr;
  }

/** extracting a single file here **/
if (ctrl_zlib->zlib_part[0]) {
  int case_sensitivity=0;
  #if (DPcSystem == DPcSystemUnix)
  case_sensitivity=1;
  #endif
  OgStringAnsiToDos(ctrl_zlib->zlib_part);
  if (unzLocateFile(uf,ctrl_zlib->zlib_part,case_sensitivity)!=UNZ_OK) {
    OgStringDosToAnsi(ctrl_zlib->zlib_part);
    sprintf(erreur,"OgZipScan: impossible to find '%s' in '%s'",ctrl_zlib->zlib_part,ctrl_zlib->zlib_name);
    OgErr(ctrl_zlib->herr,erreur); DPcErr;
    }
  IFE(OgZipScan1(ctrl_zlib,uf,ctrl_zlib->zlib_part));
  goto endOgZipScan;
  }

retour = unzGetGlobalInfo(uf,&gi);
if (retour!=UNZ_OK) {
  sprintf(erreur,"OgZipScan: unzGetGlobalInfo: error %d for '%s'",retour,ctrl_zlib->zlib_name);
  OgErr(ctrl_zlib->herr,erreur); DPcErr;
  }

for (i=0;i<(int)gi.number_entry;i++) {
  IFE(OgZipScan1(ctrl_zlib,uf,filename_inzip));
  if ((i+1)<(int)gi.number_entry) {
    retour = unzGoToNextFile(uf);
    if (retour!=UNZ_OK) {
      sprintf(erreur,"OgZipScan: unzGoToNextFile: error %d for '%s'",retour,ctrl_zlib->zlib_name);
      OgErr(ctrl_zlib->herr,erreur); DPcErr;
      }
    }
  }

endOgZipScan:

retour = unzClose(uf);
if (retour!=UNZ_OK) {
  sprintf(erreur,"OgZipScan: unzClose: error %d for '%s'",retour,ctrl_zlib->zlib_name);
  OgErr(ctrl_zlib->herr,erreur); DPcErr;
  }

IFE(OgZfileListing(ctrl_zlib));
DONE;
}



/*
 *  Extracting the file in the zip file.
*/

STATICF(int) OgZipScan1(ctrl_zlib,uf,filename_inzip)
struct og_ctrl_zlib *ctrl_zlib; unzFile uf;
char *filename_inzip;
{
char *p,file_inzip[DPcPathSize],write_filename[DPcPathSize];
unz_file_info cfile_info, *file_info=&cfile_info;
int i,slash=(-1),sbuf; void *buf;
int retour,ifile_inzip,Izfile;
struct og_zlib_file *zfile=0;
char erreur[DOgErrorSize];
time_t file_date;
FILE *fd;

retour = unzGetCurrentFileInfo(uf,file_info,filename_inzip,DPcPathSize,NULL,0,NULL,0);
if (retour!=UNZ_OK) {
  sprintf(erreur,"OgZipScan: unzGetCurrentFileInfo: error %d for '%s'",retour,ctrl_zlib->zlib_name);
  unzClose(uf); OgErr(ctrl_zlib->herr,erreur); DPcErr;
  }
/** Zip is internally DOS encoded (cp850), we decode it to ANSI (cp1252) **/
OgStringDosToAnsi(filename_inzip);

if (ctrl_zlib->loginfo->trace & DOgZlibTraceListing) {
  OgThrMessageLog(ctrl_zlib->hmutex, DOgMlogInLog, ctrl_zlib->loginfo->where, 0
    , "OgZipScan: found '%s' in '%s'", filename_inzip, ctrl_zlib->zlib_name);
  }

strcpy(file_inzip,filename_inzip); ifile_inzip=strlen(file_inzip);
/** a directory is indicated by this information **/
p=file_inzip + ifile_inzip-1;
if (*p == '/' || *p == '\\') {
  if (ctrl_zlib->loginfo->trace & DOgZlibTraceWriting) {
    OgThrMessageLog(ctrl_zlib->hmutex, DOgMlogInLog, ctrl_zlib->loginfo->where, 0
      , "OgZipScan1: directory '%s' in '%s'", filename_inzip, ctrl_zlib->zlib_name);
    }
  DONE;
  }

if (ctrl_zlib->scan_param->control_flags & DOgZlibScanFlagExtractFlat) {
  for (i=ifile_inzip-1; i>=0; i--) {
    if (file_inzip[i]=='/' || file_inzip[i]=='\\') { slash=i; break; }
    }
  if (slash>0) strcpy(file_inzip,filename_inzip+slash+1);
  }
ifile_inzip=strlen(file_inzip);

sprintf(write_filename,"%s/%s",ctrl_zlib->scan_param->directory,file_inzip);

/** Calculating listing information **/
if (ctrl_zlib->scan_param->listing) {
  IFE(Izfile=OgZipScanListingAdd(ctrl_zlib,uf,filename_inzip,file_inzip,file_info));
  zfile = ctrl_zlib->Zfile + Izfile;
  }

IFn(ctrl_zlib->scan_param->directory) DONE;

if (ctrl_zlib->loginfo->trace & DOgZlibTraceWriting) {
  OgThrMessageLog(ctrl_zlib->hmutex, DOgMlogInLog, ctrl_zlib->loginfo->where, 0
    , "OgZipScan1: writing file '%s' from '%s' into '%s'", filename_inzip, ctrl_zlib->zlib_name, write_filename);
  }

retour = unzOpenCurrentFilePassword(uf,ctrl_zlib->scan_param->password);
if (retour!=UNZ_OK) {
  sprintf(erreur,"OgZipScan1: unzOpenCurrentFilePassword: error %d for '%s' in '%s'"
    ,retour,file_inzip,ctrl_zlib->zlib_name);
  unzClose(uf); OgErr(ctrl_zlib->herr,erreur); DPcErr;
  }

/** making sure we have some room left **/
IFE(OgZlibTestReallocBa(ctrl_zlib,8192));
buf = ctrl_zlib->Ba + ctrl_zlib->BaUsed;
sbuf = ctrl_zlib->BaSize - ctrl_zlib->BaUsed;

IFE(OgCheckOrCreateDir(write_filename,1,0));
IFn(fd=fopen(write_filename,"wb")) {
  sprintf(erreur,"OgZipScan1: impossible to open '%s' for writing",write_filename);
  unzCloseCurrentFile(uf); unzClose(uf);
  OgErr(ctrl_zlib->herr,erreur); DPcErr;
  }

do {
  retour = unzReadCurrentFile(uf,buf,sbuf);
  if (retour<0) {
    /** We do not go into an error because the rest of the files may be readeable **/
    OgThrMessageLog(ctrl_zlib->hmutex, DOgMlogInLog, ctrl_zlib->loginfo->where, 0
      , "OgZipScan1: unzReadCurrentFile: error %d for '%s' in '%s'; going on"
      , retour, file_inzip, ctrl_zlib->zlib_name);
    //sprintf(erreur,"OgZipScan1: unzReadCurrentFile: error %d for '%s' in '%s'"
    //  ,retour,file_inzip,ctrl_zlib->zlib_name);
    //unzCloseCurrentFile(uf); unzClose(uf); fclose(fd);
    //OgErr(ctrl_zlib->herr,erreur); DPcErr;
    break;
    }
  if (retour>0) {
    if (fwrite(buf,retour,1,fd)!=1) {
      sprintf(erreur,"OgZipScan1: error fwrite '%s' from '%s' in '%s'"
        ,write_filename,file_inzip,ctrl_zlib->zlib_name);
      unzCloseCurrentFile(uf); unzClose(uf); fclose(fd);
      OgErr(ctrl_zlib->herr,erreur); DPcErr;
      }
    }
  }
while (retour>0);

fclose(fd);

retour = unzCloseCurrentFile(uf);
if (retour!=UNZ_OK) {
  sprintf(erreur,"OgZipScan1: unzCloseCurrentFile: error %d for '%s' in '%s'"
    ,retour,file_inzip,ctrl_zlib->zlib_name);
  unzClose(uf); OgErr(ctrl_zlib->herr,erreur); DPcErr;
  }

if (zfile) {
  /** Changing the date on the file **/
  IF(OgDateToTime_t(&zfile->date,&file_date)) {
    OgErrLast(0,erreur,0);
    }
  else {
    IF(OgChangeDate(ctrl_zlib->herr,write_filename,file_date)) {
      OgErrLast(ctrl_zlib->herr,erreur,0);
      }
    }
  }

DONE;
}





STATICF(int) OgZipScanListingAdd(ctrl_zlib,uf,filename_inzip,file_inzip,file_info)
struct og_ctrl_zlib *ctrl_zlib; unzFile uf;
char *filename_inzip, *file_inzip;
unz_file_info *file_info;
{
int ibuffer; unsigned char buffer[DPcPathSize];
int c,ifilename_inzip=strlen(filename_inzip);
int ifile_inzip=strlen(file_inzip);
struct og_zlib_file *zfile;
struct og_date *date;
int i,Izfile;

IFE(Izfile=GetZfile(ctrl_zlib,&zfile));

c=filename_inzip[ifilename_inzip-1];
if (c == '/' || c == '\\') zfile->is_dir=1;

date = &zfile->date;

date->iyear = file_info->tmu_date.tm_year;
date->month = file_info->tmu_date.tm_mon+1;
date->mday = file_info->tmu_date.tm_mday;
date->hour = file_info->tmu_date.tm_hour;
date->minute = file_info->tmu_date.tm_min;
date->isecond = file_info->tmu_date.tm_sec;

zfile->compressed_size = file_info->compressed_size;
zfile->uncompressed_size = file_info->uncompressed_size;

zfile->original_name = ctrl_zlib->Ba + ctrl_zlib->BaUsed;
IFE(OgZlibAppendBa(ctrl_zlib,ifilename_inzip,filename_inzip));
ctrl_zlib->Ba[ctrl_zlib->BaUsed++]=0;

/* original_name is the untouched extracted name and
 * written_name is the modified name for compatibility.
 * For example a zip file coming from Macintosh can contain
 * CR within it. We replace such character with a space. */
memcpy(buffer,file_inzip,ifile_inzip); buffer[ifile_inzip]=0;
for (i=0; i<ifile_inzip; i++) {
  if (buffer[i]==':') buffer[i]=' ';
  if (buffer[i]<32)
    buffer[i]=' ';
  }
OgTrimString(buffer,buffer); ibuffer=strlen(buffer);

zfile->written_name = ctrl_zlib->Ba + ctrl_zlib->BaUsed;
IFE(OgZlibAppendBa(ctrl_zlib,ibuffer,buffer));
ctrl_zlib->Ba[ctrl_zlib->BaUsed++]=0;

return(Izfile);
}

